#include "base.h"
#include <pthread.h>
#include <unistd.h>

/*
 * constants
 */

#define N_THREADS 6

/*
 * tables
 */

#define NODE_KINDS \
X(BLOCK) \
X(TEXT) \
X(REFERENCE) \
X(INPUT) \

typedef enum Node_Kind {
  NODE_KIND_NULL = 0,
  #define X(kind) NODE_KIND_##kind,
  NODE_KINDS
  #undef X
  NODE_KIND_COUNT,
} Node_Kind;

/*
 * types
 */

typedef struct Node Node;
typedef struct Worker Worker;
typedef struct Job_Queue Job_Queue;
typedef struct Job Job;
typedef struct Job_Node Job_Node;
typedef struct Job_List Job_List;
typedef struct Symbol Symbol;
typedef struct Symbol_Table Symbol_Table;

/*
 * struct bodies
 */

DECL_ARR_TYPE_NAME(Node*, Arr_Node_Ptr);

struct Worker {
  Str8 name;
  Arena *scratch;
  u32 id;
};

struct Node {
  Node_Kind kind;
  Node *next;
  Node *parent;
  Node *down;
  Str8 block_name;
  Str8 ref_name;
  Str8 text;
};

struct Job_Queue {
  s64 count;
  pthread_mutex_t mutex;
  Job *first;
  Job *last;
};

struct Job_Node {
  Job_Node *next;
  Job *job;
};

struct Job_List {
  Job_Node *first;
  Job_Node *last;
  s64 count;
};

struct Job {
  Arena *scratch;
  Node *node;
  Node *cur_node;
  b32 complete;
  u32 id;
  Str8_List chunks;
  Str8 result;
  Job *next;
  Str8 handling_name;
  Str8 waiting_on_name;
  s64 resubmition_count;
  s64 last_number_of_resubmitted_jobs;
};

struct Symbol {
  Str8 key;
  Str8 text;
};

DECL_ARR_TYPE(Symbol);

struct Symbol_Table {
  Arr_Symbol table;
  pthread_mutex_t mutex;

};

/*
 * headers
 */

Node* parse_text(Arena *a, Str8 text);
Node* push_node(Arena *a);

void* worker(void *arg);
int main(void);

void job_queue_init(Job_Queue *queue);
void job_push(Job_Queue *queue, Job *job);
Job* job_pop(Job_Queue *queue);

void      symbol_table_init(Symbol_Table *symbol_table, Arena *arena);
void      symbol_save_text(Str8 key, Str8 text);
Symbol*   symbol_get(Str8 key);
Symbol*   symbol_get_lockless(Str8 key);
void      symbol_table_lock(Symbol_Table *symbol_table);
void      symbol_table_unlock(Symbol_Table *symbol_table);

/*
 * globals
 */

pthread_mutex_t scratch_mutex = PTHREAD_MUTEX_INITIALIZER;
Symbol_Table symbol_table;
Job_Queue job_queue;
Job_Queue completed_job_queue;
Job_Queue dependency_problem_job_queue;
s64 number_of_resubmitted_jobs;
Arena *scratch;

/*
 * functions
 */

void* worker_run(void *arg) {
  Worker *worker = (Worker*)arg;

  printf("Worker %u starting...\n", worker->id);

  for(Job *job = job_pop(&job_queue); job; job = job_pop(&job_queue)) {
    arena_clear(worker->scratch);

    Node *node = job->node;

    printf("Worker %u executing job %u...\n", worker->id, job->id);

    ASSERT(node->kind == NODE_KIND_BLOCK);

    Str8 block_name = node->block_name;

    if(job->cur_node) {
      node = job->cur_node;
    } else {
      job->handling_name = block_name;
      node = node->down;
    }

    b32 skip_job = false;
    for(; node; node = node->next) {
      switch(node->kind) {
        case NODE_KIND_TEXT: {
          str8_list_append_str(job->scratch, job->chunks, node->text);
        } break;
        case NODE_KIND_REFERENCE: {
          Str8 ref_name = node->ref_name;

          Str8 symbol_text;
          // defer_loop(symbol_table_lock(&symbol_table), symbol_table_unlock(&symbol_table)) {

          // Symbol *symbol = symbol_get_lockless(ref_name);
          Symbol *symbol = symbol_get(ref_name);

          if(!symbol) {

            s64 cur_number_of_resubmitted_jobs = __atomic_load_n(&number_of_resubmitted_jobs, __ATOMIC_RELAXED);
            if(job->resubmition_count > 6 && job->last_number_of_resubmitted_jobs <= cur_number_of_resubmitted_jobs) {
              printf("dependency problem in job %u...\n", job->id);
              job_push(&dependency_problem_job_queue, job);
              job->waiting_on_name = ref_name;
              skip_job = true;
            } else {
              job->waiting_on_name = ref_name;
              skip_job = true;
              job->cur_node = node;
              job->last_number_of_resubmitted_jobs = __atomic_add_fetch(&number_of_resubmitted_jobs, 1, __ATOMIC_SEQ_CST);
              job->resubmition_count++;
              job_push(&job_queue, job);
            }

          } else {
            symbol_text = symbol->text;
            str8_list_append_str(job->scratch, job->chunks, symbol_text);
          }

          // }

          if(skip_job) {
            goto skip_job;
          }

        } break;
        default: UNREACHABLE;
      }
    }

    skip_job:
    if(skip_job) {
      continue;
    }

    Str8 text;
    defer_loop(pthread_mutex_lock(&scratch_mutex), pthread_mutex_unlock(&scratch_mutex)) {
      text = str8_list_join(scratch, job->chunks, str8_lit(". "));
      symbol_save_text(block_name, text);
    }

    job->result = text;
    job->complete = true;

    s64 cur_number_of_resubmitted_jobs = __atomic_load_n(&number_of_resubmitted_jobs, __ATOMIC_RELAXED);
    if(cur_number_of_resubmitted_jobs > 0) {
      __atomic_compare_exchange_n(
        &number_of_resubmitted_jobs,
        &cur_number_of_resubmitted_jobs,
        cur_number_of_resubmitted_jobs - 1,
        0,
        __ATOMIC_ACQ_REL,
        __ATOMIC_RELAXED);
    }

    job_push(&completed_job_queue, job);

  }

  return 0;
}

void symbol_table_init(Symbol_Table *symbol_table, Arena *arena) {
  arr_init_ex(symbol_table->table, arena, 512);
  pthread_mutex_init(&symbol_table->mutex, 0);
}

void symbol_table_lock(Symbol_Table *symbol_table) {
  pthread_mutex_lock(&symbol_table->mutex);
}

void symbol_table_unlock(Symbol_Table *symbol_table) {
  pthread_mutex_unlock(&symbol_table->mutex);
}

void symbol_save_text(Str8 key, Str8 text) {
  defer_loop(symbol_table_lock(&symbol_table), symbol_table_unlock(&symbol_table)) {
    s64 index = arr_dict_put_(
      header_ptr_from_arr(symbol_table.table),
      arr_stride(symbol_table.table),
      member_offset(Symbol, key),
      key);
    symbol_table.table.d[index].key = key;
    symbol_table.table.d[index].text = text;
  }
}

Symbol* symbol_get(Str8 key) {
  Symbol *result = 0;
  defer_loop(symbol_table_lock(&symbol_table), symbol_table_unlock(&symbol_table)) {
    result = symbol_get_lockless(key);
  }
  return result;
}

Symbol* symbol_get_lockless(Str8 key) {
  Symbol *result = 0;
  s64 index = arr_dict_get_(
    header_ptr_from_arr(symbol_table.table),
    arr_stride(symbol_table.table),
    member_offset(Symbol, key),
    key);
  if(index >= 0) {
    result = &symbol_table.table.d[index];
  }
  return result;
}

void job_push(Job_Queue *queue, Job *job) {
  defer_loop(pthread_mutex_lock(&queue->mutex), pthread_mutex_unlock(&queue->mutex)) {
    sll_queue_push(queue->first, queue->last, job);
    queue->count++;
  }
}

Job* job_pop(Job_Queue *queue) {
  Job *job = 0;
  defer_loop(pthread_mutex_lock(&queue->mutex), pthread_mutex_unlock(&queue->mutex)) {
    job = queue->first;
    sll_queue_pop(queue->first, queue->last);
    queue->count--;
  }
  return job;
}

void job_queue_init(Job_Queue *queue) {
  pthread_mutex_init(&queue->mutex, 0);
}

Node* push_node(Arena *a) {
  Node *node = arena_push(a, sizeof(Node), alignof(Node));
  memory_zero(node, sizeof(Node));
  return node;
}

int main(void) {
  scratch = arena_create(MB(1));

  symbol_table_init(&symbol_table, scratch);

  Str8 file_text = os_read_entire_file(scratch, str8_lit("hello.txt"));

  printf("parsing file...\n");

  Node *root = parse_text(scratch, file_text);
  ASSERT(root);

  job_queue_init(&job_queue);
  job_queue_init(&completed_job_queue);
  job_queue_init(&dependency_problem_job_queue);

  u32 job_id = 0;
  for(Node *node = root; node; node = node->next) {
    Job *job = push_struct(scratch, Job);
    job->id = job_id++;
    job->node = node;
    job->last_number_of_resubmitted_jobs = 1;
    job->scratch = arena_create(256);
    job_queue.count++;
    sll_queue_push(job_queue.first, job_queue.last, job);
  }
  printf("job_id = %u\n", job_id);
  u32 n_threads = N_THREADS;

  Arr(pthread_t) threads;
  arr_init_ex(threads, scratch, n_threads);

  Arr(Worker) workers;
  arr_init_ex(workers, scratch, n_threads);
  for(u32 i = 0; i < n_threads; i++) {
    Worker worker = {
      .name = str8f(scratch, "Worker %i", i),
      .scratch = arena_create(KB(100)),
      .id = i,
    };
    arr_push(workers, worker);
  }

  for(u32 i = 0; i < n_threads; i++) {
    if(pthread_create(&threads.d[i], 0, worker_run, (void*)&workers.d[i]) != 0) {
      ASSERT(0);
    }
  }

  for(u32 i = 0; i < n_threads; i++) {
    pthread_join(threads.d[i], 0);
  }

  for(Job *job = dependency_problem_job_queue.first; job; job = job->next) {
    printf("===========\n\n");
    printf("job.id = %u\njob.handling_name = %.*s\njob.waiting_on_name = %.*s\n", job->id, (int)job->handling_name.len, (char*)job->handling_name.s, (int)job->waiting_on_name.len, (char*)job->waiting_on_name.s);
  }

  printf("\n\njobs with dependency problems: %li\n\n", dependency_problem_job_queue.count);

  for(Job *job = completed_job_queue.first; job; job = job->next) {
    if(str8_match_lit("play", job->node->block_name)) {
      printf("\n%.*s\n", (int)job->result.len, (char*)job->result.s);
    }
  }

  return 0;
}

Node* parse_text(Arena *a, Str8 text) {
  s64 pos = 0;
  b32 inside_block = false;

  Node *file_nodes_first = 0;
  Node *file_nodes_last = 0;
  Node *block_nodes_first = 0;
  Node *block_nodes_last = 0;

  while(pos < text.len) {

    while(pos < text.len && is_space(text.s[pos])) {
      pos++;
    }

    Str8 cur_slice = str8_slice(text, pos, -1);

    if(inside_block) {
      if(cur_slice.s[0] == '@') {
        Node *ref_node = push_node(a);
        ref_node->kind = NODE_KIND_REFERENCE;
        s64 first_dot = str8_find_char(cur_slice, '.');
        s64 first_whitespace = str8_find_first_whitespace(cur_slice);
        s64 name_boundary_index;
        if(first_dot < 0) {
          name_boundary_index = first_whitespace;
        } else if(first_whitespace < 0) {
          name_boundary_index = first_dot;
        } else {
          name_boundary_index = MIN(first_dot, first_whitespace);
        }
        Str8 refname = str8_slice(cur_slice, 1, name_boundary_index);
        ref_node->ref_name = refname;
        ref_node->parent = file_nodes_last;

        sll_queue_push(block_nodes_first, block_nodes_last, ref_node);

        name_boundary_index++;
        pos += name_boundary_index;
        continue;
      } else if(cur_slice.s[0] == '}') {
        file_nodes_last->down = block_nodes_first;
        block_nodes_first = 0;
        block_nodes_last = 0;
        pos++;
        inside_block = false;
        continue;

      } else {
        s64 text_block_cursor = 0;
        s64 start_text_block = 0;

        u8 c = cur_slice.s[text_block_cursor];
        while(c != '.' && c != '@' && c != '}') {
          text_block_cursor++;
          c = cur_slice.s[text_block_cursor];
        }

        Node *text_node = push_node(a);
        text_node->kind = NODE_KIND_TEXT;
        text_node->text = str8_slice(cur_slice, start_text_block, text_block_cursor);
        text_node->parent = file_nodes_last;

        sll_queue_push(block_nodes_first, block_nodes_last, text_node);

        if(cur_slice.s[text_block_cursor] == '.') {
          text_block_cursor++;
        }

        pos += text_block_cursor;
        continue;
      }
    } else {
      if(cur_slice.s[0] == '@') {

        Node *block_node = push_node(a);
        block_node->kind = NODE_KIND_BLOCK;
        s64 first_space_index = str8_find_first_whitespace(cur_slice);
        Str8 block_name = str8_slice(cur_slice, 1, first_space_index);
        block_node->block_name = block_name;

        sll_queue_push(file_nodes_first, file_nodes_last, block_node);

        pos += first_space_index;

        while(pos < text.len && text.s[pos] != '{') {
          pos++;
        }

        ASSERT(text.s[pos] == '{');
        pos++;

        inside_block = true;

        continue;
      }
    }

  }

  return file_nodes_first;
}
