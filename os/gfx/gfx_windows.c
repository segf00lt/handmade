#ifndef GFX_WINDOWS_C
#define GFX_WINDOWS_C


////////////////////////////////////////
// GFX

KeyboardModifier os_get_modifiers(void) {
  KeyboardModifier modifier_mask = 0;

  if(GetKeyState(VK_CONTROL) & 0x8000) {
    modifier_mask |= KBD_MOD_CONTROL;
  }

  if(GetKeyState(VK_SHIFT) & 0x8000) {
    modifier_mask |= KBD_MOD_SHIFT;
  }

  if(GetKeyState(VK_MENU) & 0x8000) {
    modifier_mask |= KBD_MOD_ALT;
  }

  if((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) {
    modifier_mask |= KBD_MOD_META;
  }

  return modifier_mask;
}


////////////////////////////////////////
// OS_Win32

OS_Event* os_win32_event_push(Arena *a, OS_EventList *event_list, OS_EventKind event_kind) {
  OS_Event *event = push_struct_no_zero(a, OS_Event);
  event->kind = event_kind;
  event->modifier_mask = os_get_modifiers();
  sll_queue_push(event_list->first, event_list->last, event);
  event_list->count++;
  return event;
}


OS_Event* os_win32_event_pop(OS_EventList *event_list) {
  OS_Event *event = event_list->last;
  sll_queue_pop(event_list->first, event_list->last);
  event_list->count--;
  return event;
}


MouseButton os_win32_mouse_button_from_virtual_keycode(WPARAM virtual_keycode) {
  MouseButton button = 0;

  switch(virtual_keycode) {
    case VK_LBUTTON /* 0x01	Left mouse button */:
    button = MOUSE_BUTTON_LEFT;
    break;
    case VK_RBUTTON /* 0x02	Right mouse button */:
    button = MOUSE_BUTTON_RIGHT;
    break;
    case VK_MBUTTON /* 0x04	Middle mouse button */:
    button = MOUSE_BUTTON_MIDDLE;
    break;
  }

  return button;
}

KeyboardKey os_win32_keyboard_key_from_virtual_keycode(WPARAM virtual_keycode) {
  KeyboardKey key = 0;

  switch(virtual_keycode) {


    case VK_CANCEL /* 0x03	Control-break processing */:
    break;
    case VK_XBUTTON1 /* 0x05	X1 mouse button */:
    break;
    case VK_XBUTTON2 /* 0x06	X2 mouse button */:
    break;
    // 0x07	Reserved
    case VK_BACK /* 0x08	Backspace key */:
    key = KBD_KEY_BACKSPACE;
    break;
    case VK_TAB /* 0x09	Tab key */:
    key = KBD_KEY_TAB;
    break;
    // 0x0A-0B	Reserved
    case VK_CLEAR /* 0x0C	Clear key */:
    break;
    case VK_RETURN /* 0x0D	Enter key */:
    key = KBD_KEY_ENTER;
    break;
    // 0x0E-0F	Unassigned
    case VK_SHIFT /* 0x10	Shift key */:
    key = KBD_KEY_LEFT_SHIFT;
    break;
    case VK_CONTROL /* 0x11	Ctrl key */:
    key = KBD_KEY_LEFT_CONTROL;
    break;
    case VK_MENU /* 0x12	Alt key */:
    key = KBD_KEY_LEFT_ALT;
    break;
    case VK_PAUSE /* 0x13	Pause key */:
    break;
    case VK_CAPITAL /* 0x14	Caps lock key */:
    key = KBD_KEY_CAPS_LOCK;
    break;
    case VK_ESCAPE /* 0x1B	Esc key */:
    key = KBD_KEY_ESCAPE;
    break;
    case VK_SPACE /* 0x20	Spacebar key */:
    key = KBD_KEY_SPACE;
    break;
    case VK_PRIOR /* 0x21	Page up key */:
    key = KBD_KEY_PAGE_UP;
    break;
    case VK_NEXT /* 0x22	Page down key */:
    key = KBD_KEY_PAGE_DOWN;
    break;
    case VK_END /* 0x23	End key */:
    key = KBD_KEY_END;
    break;
    case VK_HOME /* 0x24	Home key */:
    key = KBD_KEY_HOME;
    break;
    case VK_LEFT /* 0x25	Left arrow key */:
    key = KBD_KEY_LEFT_ARROW;
    break;
    case VK_UP /* 0x26	Up arrow key */:
    key = KBD_KEY_UP_ARROW;
    break;
    case VK_RIGHT /* 0x27	Right arrow key */:
    key = KBD_KEY_RIGHT_ARROW;
    break;
    case VK_DOWN /* 0x28	Down arrow key */:
    key = KBD_KEY_DOWN_ARROW;
    break;
    case VK_SNAPSHOT /* 0x2C	Print screen key */:
    key = KBD_KEY_PRINT_SCREEN;
    break;
    case VK_INSERT /* 0x2D	Insert key */:
    key = KBD_KEY_INSERT;
    break;
    case VK_DELETE /* 0x2E	Delete key */:
    key = KBD_KEY_DELETE;
    break;
    case 0x30 /* 0 key */:
    key = KBD_KEY_0;
    break;
    case 0x31 /* 1 key */:
    key = KBD_KEY_1;
    break;
    case 0x32 /* 2 key */:
    key = KBD_KEY_2;
    break;
    case 0x33 /* 3 key */:
    key = KBD_KEY_3;
    break;
    case 0x34 /* 4 key */:
    key = KBD_KEY_4;
    break;
    case 0x35 /* 5 key */:
    key = KBD_KEY_5;
    break;
    case 0x36 /* 6 key */:
    key = KBD_KEY_6;
    break;
    case 0x37 /* 7 key */:
    key = KBD_KEY_7;
    break;
    case 0x38 /* 8 key */:
    key = KBD_KEY_8;
    break;
    case 0x39 /* 9 key */:
    key = KBD_KEY_9;
    break;
    // 0x3A-40	Undefined
    case 0x41 /* A key */:
    key = KBD_KEY_A;
    break;
    case 0x42 /* B key */:
    key = KBD_KEY_B;
    break;
    case 0x43 /* C key */:
    key = KBD_KEY_C;
    break;
    case 0x44 /* D key */:
    key = KBD_KEY_D;
    break;
    case 0x45 /* E key */:
    key = KBD_KEY_E;
    break;
    case 0x46 /* F key */:
    key = KBD_KEY_F;
    break;
    case 0x47 /* G key */:
    key = KBD_KEY_G;
    break;
    case 0x48 /* H key */:
    key = KBD_KEY_H;
    break;
    case 0x49 /* I key */:
    key = KBD_KEY_I;
    break;
    case 0x4A /* J key */:
    key = KBD_KEY_J;
    break;
    case 0x4B /* K key */:
    key = KBD_KEY_K;
    break;
    case 0x4C /* L key */:
    key = KBD_KEY_L;
    break;
    case 0x4D /* M key */:
    key = KBD_KEY_M;
    break;
    case 0x4E /* N key */:
    key = KBD_KEY_N;
    break;
    case 0x4F /* O key */:
    key = KBD_KEY_O;
    break;
    case 0x50 /* P key */:
    key = KBD_KEY_P;
    break;
    case 0x51 /* Q key */:
    key = KBD_KEY_Q;
    break;
    case 0x52 /* R key */:
    key = KBD_KEY_R;
    break;
    case 0x53 /* S key */:
    key = KBD_KEY_S;
    break;
    case 0x54 /* T key */:
    key = KBD_KEY_T;
    break;
    case 0x55 /* U key */:
    key = KBD_KEY_U;
    break;
    case 0x56 /* V key */:
    key = KBD_KEY_V;
    break;
    case 0x57 /* W key */:
    key = KBD_KEY_W;
    break;
    case 0x58 /* X key */:
    key = KBD_KEY_X;
    break;
    case 0x59 /* Y key */:
    key = KBD_KEY_Y;
    break;
    case 0x5A /* Z key */:
    key = KBD_KEY_Z;
    break;
    case VK_LWIN /* 0x5B	Left Windows logo key */:
    key = KBD_KEY_LEFT_META;
    break;
    case VK_RWIN /* 0x5C	Right Windows logo key */:
    key = KBD_KEY_RIGHT_META;
    break;
    // 0x5E	Reserved
    case VK_NUMPAD0 /* 0x60	Numeric keypad 0 key */:
    key = KBD_KEY_0;
    break;
    case VK_NUMPAD1 /* 0x61	Numeric keypad 1 key */:
    key = KBD_KEY_1;
    break;
    case VK_NUMPAD2 /* 0x62	Numeric keypad 2 key */:
    key = KBD_KEY_2;
    break;
    case VK_NUMPAD3 /* 0x63	Numeric keypad 3 key */:
    key = KBD_KEY_3;
    break;
    case VK_NUMPAD4 /* 0x64	Numeric keypad 4 key */:
    key = KBD_KEY_4;
    break;
    case VK_NUMPAD5 /* 0x65	Numeric keypad 5 key */:
    key = KBD_KEY_5;
    break;
    case VK_NUMPAD6 /* 0x66	Numeric keypad 6 key */:
    key = KBD_KEY_6;
    break;
    case VK_NUMPAD7 /* 0x67	Numeric keypad 7 key */:
    key = KBD_KEY_7;
    break;
    case VK_NUMPAD8 /* 0x68	Numeric keypad 8 key */:
    key = KBD_KEY_8;
    break;
    case VK_NUMPAD9 /* 0x69	Numeric keypad 9 key */:
    key = KBD_KEY_9;
    break;

    case VK_F1 /* 0x70	F1 key */:
    key = KBD_KEY_F1;
    break;
    case VK_F2 /* 0x71	F2 key */:
    key = KBD_KEY_F2;
    break;
    case VK_F3 /* 0x72	F3 key */:
    key = KBD_KEY_F3;
    break;
    case VK_F4 /* 0x73	F4 key */:
    key = KBD_KEY_F4;
    break;
    case VK_F5 /* 0x74	F5 key */:
    key = KBD_KEY_F5;
    break;
    case VK_F6 /* 0x75	F6 key */:
    key = KBD_KEY_F6;
    break;
    case VK_F7 /* 0x76	F7 key */:
    key = KBD_KEY_F7;
    break;
    case VK_F8 /* 0x77	F8 key */:
    key = KBD_KEY_F8;
    break;
    case VK_F9 /* 0x78	F9 key */:
    key = KBD_KEY_F9;
    break;
    case VK_F10 /* 0x79	F10 key */:
    key = KBD_KEY_F10;
    break;
    case VK_F11 /* 0x7A	F11 key */:
    key = KBD_KEY_F11;
    break;
    case VK_F12 /* 0x7B	F12 key */:
    key = KBD_KEY_F12;
    break;
    // 0x88-8F	Reserved
    case VK_SCROLL /* 0x91	Scroll lock key */:
    break;
    // 0x92-96	OEM specific
    // 0x97-9F	Unassigned
    case VK_LSHIFT /* 0xA0	Left Shift key */:
    key = KBD_KEY_LEFT_SHIFT;
    break;
    case VK_RSHIFT /* 0xA1	Right Shift key */:
    key = KBD_KEY_RIGHT_SHIFT;
    break;
    case VK_LCONTROL /* 0xA2	Left Ctrl key */:
    key = KBD_KEY_LEFT_CONTROL;
    break;
    case VK_RCONTROL /* 0xA3	Right Ctrl key */:
    key = KBD_KEY_RIGHT_CONTROL;
    break;
    case VK_LMENU /* 0xA4	Left Alt key */:
    key = KBD_KEY_LEFT_ALT;
    break;
    case VK_RMENU /* 0xA5	Right Alt key */:
    key = KBD_KEY_RIGHT_ALT;
    break;
    // 0xB8-B9	Reserved
    case VK_OEM_1 /* 0xBA	It can vary by keyboard. For the US ANSI keyboard , the Semiсolon and Colon key */:
    break;
    case VK_OEM_PLUS /* 0xBB	For any country/region, the Equals and Plus key */:
    break;
    case VK_OEM_COMMA /* 0xBC	For any country/region, the Comma and Less Than key */:
    break;
    case VK_OEM_MINUS /* 0xBD	For any country/region, the Dash and Underscore key */:
    break;
    case VK_OEM_PERIOD /* 0xBE	For any country/region, the Period and Greater Than key */:
    break;
    case VK_OEM_2 /* 0xBF	It can vary by keyboard. For the US ANSI keyboard, the Forward Slash and Question Mark key */:
    break;
    case VK_OEM_3 /* 0xC0	It can vary by keyboard. For the US ANSI keyboard, the Grave Accent and Tilde key */:
    break;
    case VK_OEM_4 /* 0xDB	It can vary by keyboard. For the US ANSI keyboard, the Left Brace key */:
    break;
    case VK_OEM_5 /* 0xDC	It can vary by keyboard. For the US ANSI keyboard, the Backslash and Pipe key */:
    break;
    case VK_OEM_6 /* 0xDD	It can vary by keyboard. For the US ANSI keyboard, the Right Brace key */:
    break;
    case VK_OEM_7 /* 0xDE	It can vary by keyboard. For the US ANSI keyboard, the Apgfxtrophe and Double Quotation Mark key */:
    break;
    case VK_OEM_8 /* 0xDF	It can vary by keyboard. For the Canadian CSA keyboard, the Right Ctrl key */:
    break;
    // 0xE0	Reserved
    // 0xE1	OEM specific
    case VK_OEM_102 /* 0xE2	It can vary by keyboard. For the European ISO keyboard, the Backslash and Pipe key */:
    break;
    // 0xC1-C2	Reserved
    case VK_GAMEPAD_A /* 0xC3	Gamepad A button */:
    break;
    case VK_GAMEPAD_B /* 0xC4	Gamepad B button */:
    break;
    case VK_GAMEPAD_X /* 0xC5	Gamepad X button */:
    break;
    case VK_GAMEPAD_Y /* 0xC6	Gamepad Y button */:
    break;
    case VK_GAMEPAD_RIGHT_SHOULDER /* 0xC7	Gamepad Right Shoulder button */:
    break;
    case VK_GAMEPAD_LEFT_SHOULDER /* 0xC8	Gamepad Left Shoulder button */:
    break;
    case VK_GAMEPAD_LEFT_TRIGGER /* 0xC9	Gamepad Left Trigger button */:
    break;
    case VK_GAMEPAD_RIGHT_TRIGGER /* 0xCA	Gamepad Right Trigger button */:
    break;
    case VK_GAMEPAD_DPAD_UP /* 0xCB	Gamepad D-pad Up button */:
    break;
    case VK_GAMEPAD_DPAD_DOWN /* 0xCC	Gamepad D-pad Down button */:
    break;
    case VK_GAMEPAD_DPAD_LEFT /* 0xCD	Gamepad D-pad Left button */:
    break;
    case VK_GAMEPAD_DPAD_RIGHT /* 0xCE	Gamepad D-pad Right button */:
    break;
    case VK_GAMEPAD_MENU /* 0xCF	Gamepad Menu/Start button */:
    break;
    case VK_GAMEPAD_VIEW /* 0xD0	Gamepad View/Back button */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON /* 0xD1	Gamepad Left Thumbstick button */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON /* 0xD2	Gamepad Right Thumbstick button */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP /* 0xD3	Gamepad Left Thumbstick up */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN /* 0xD4	Gamepad Left Thumbstick down */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT /* 0xD5	Gamepad Left Thumbstick right */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT /* 0xD6	Gamepad Left Thumbstick left */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP /* 0xD7	Gamepad Right Thumbstick up */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN /* 0xD8	Gamepad Right Thumbstick down */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT /* 0xD9	Gamepad Right Thumbstick right */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT /* 0xDA	Gamepad Right Thumbstick left */:
    break;
    // 0xE6	OEM specific
    case VK_PACKET /* 0xE7	Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP */:
    break;
    // 0xE8	Unassigned
  }

  return key;
}


#endif
