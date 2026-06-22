#include "gamepad_window.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <windowsx.h>
#include <string.h> // Required for _stricmp

#include <ViGEm/Client.h>

#include "utils.h"

#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define BUTTON(HWND, VAR) \
	Button* VAR = (Button*)GetWindowLongPtr(HWND, GWLP_USERDATA);

// EXTERN VIGEM STATE
extern PVIGEM_CLIENT g_client;
extern PVIGEM_TARGET g_pad;
extern XUSB_REPORT g_report;

typedef enum
{
	TOUCH_DOWN,
	TOUCH_UP,
	TOUCH_MOVE
} TouchEvent;

LRESULT CALLBACK Paint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(uMsg);
	UNUSED(wParam);
	UNUSED(lParam);

	BUTTON(hWnd, button);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	HDC buttonDC = CreateCompatibleDC(hdc);
	SelectObject(buttonDC, button->image);
	BitBlt(hdc, 0, 0, button->width, button->height, buttonDC, 0, 0, SRCCOPY);
	DeleteDC(buttonDC);
	EndPaint(hWnd, &ps);

	return 0;
}

void HandleKeyButton(Button* button, bool down)
{
	KEYBDINPUT kbInput;
	kbInput.wVk = button->extras.key.code;
	kbInput.wScan = 0;
	kbInput.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
	kbInput.time = 0;
	kbInput.dwExtraInfo = 0;

	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki = kbInput;

	SendInput(1, &input, sizeof(INPUT));
}

// --- NEW XBOX BUTTON MAPPING ---
USHORT GetXboxButtonMask(const char* name)
{
	if (_stricmp(name, "a") == 0) return XUSB_GAMEPAD_A;
	if (_stricmp(name, "b") == 0) return XUSB_GAMEPAD_B;
	if (_stricmp(name, "x") == 0) return XUSB_GAMEPAD_X;
	if (_stricmp(name, "y") == 0) return XUSB_GAMEPAD_Y;
	if (_stricmp(name, "lb") == 0 || _stricmp(name, "l") == 0) return XUSB_GAMEPAD_LEFT_SHOULDER;
	if (_stricmp(name, "rb") == 0 || _stricmp(name, "r") == 0) return XUSB_GAMEPAD_RIGHT_SHOULDER;
	if (_stricmp(name, "start") == 0) return XUSB_GAMEPAD_START;
	if (_stricmp(name, "select") == 0 || _stricmp(name, "back") == 0) return XUSB_GAMEPAD_BACK;
	if (_stricmp(name, "up") == 0) return XUSB_GAMEPAD_DPAD_UP;
	if (_stricmp(name, "down") == 0) return XUSB_GAMEPAD_DPAD_DOWN;
	if (_stricmp(name, "left") == 0) return XUSB_GAMEPAD_DPAD_LEFT;
	if (_stricmp(name, "right") == 0) return XUSB_GAMEPAD_DPAD_RIGHT;
	if (_stricmp(name, "ls") == 0 || _stricmp(name, "l3") == 0) return XUSB_GAMEPAD_LEFT_THUMB;
	if (_stricmp(name, "rs") == 0 || _stricmp(name, "r3") == 0) return XUSB_GAMEPAD_RIGHT_THUMB;
	return 0;
}

void HandleXboxButton(Button* button, bool down)
{
	// Handle Analog Triggers (LT / RT)
	if (_stricmp(button->name, "lt") == 0) {
		g_report.bLeftTrigger = down ? 255 : 0;
	}
	else if (_stricmp(button->name, "rt") == 0) {
		g_report.bRightTrigger = down ? 255 : 0;
	}
	else {
		// Handle Digital Buttons
		USHORT mask = GetXboxButtonMask(button->name);
		if (mask != 0) {
			if (down) g_report.wButtons |= mask;
			else      g_report.wButtons &= ~mask;
		}
		else {
			// Fallback: If it's not a standard Xbox name, send a normal keyboard key
			HandleKeyButton(button, down);
			return;
		}
	}

	// Push the updated state to the virtual controller
	vigem_target_x360_update(g_client, g_pad, g_report);
}
// -------------------------------

void HandleQuitButton(Button* button, bool down)
{
	UNUSED(button);

	if (!down) { PostQuitMessage(0); }
}

void HandleWheelButton(Button* button, bool down)
{
	if (!down) { return; }

	INPUT inputs[2];

	inputs[0].type = INPUT_MOUSE;
	int x = GetButtonX(button) - 5;
	int y = GetButtonY(button) - 5;
	int absX = (int)((float)x / (float)GetSystemMetrics(SM_CXSCREEN) * 65535.f);
	int absY = (int)((float)y / (float)GetSystemMetrics(SM_CYSCREEN) * 65535.f);
	inputs[0].mi.dx = absX;
	inputs[0].mi.dy = absY;
	inputs[0].mi.mouseData = 0;
	inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	inputs[0].mi.time = 0;
	inputs[0].mi.dwExtraInfo = 0;

	inputs[1].type = INPUT_MOUSE;
	inputs[1].mi.dx = 0;
	inputs[1].mi.dy = 0;
	inputs[1].mi.mouseData = WHEEL_DELTA * button->extras.wheel.direction * button->extras.wheel.amount;
	inputs[1].mi.dwFlags = MOUSEEVENTF_WHEEL;
	inputs[1].mi.time = 0;
	inputs[1].mi.dwExtraInfo = 0;

	SendInput(2, inputs, sizeof(INPUT));
}

void HandleStickButton(Button* button, TouchEvent event, int touchX, int touchY)
{
	float joyX = 0.0f;
	float joyY = 0.0f;

	if (event != TOUCH_UP)
	{
		joyX = (float)touchX / (float)button->width * 2.0f - 1.0f;
		joyY = (float)touchY / (float)button->height * 2.0f - 1.0f;

		if (joyX < -1.0f) joyX = -1.0f;
		if (joyX > 1.0f) joyX = 1.0f;
		if (joyY < -1.0f) joyY = -1.0f;
		if (joyY > 1.0f) joyY = 1.0f;
	}

	g_report.sThumbLX = (SHORT)(joyX * 32767.0f);
	g_report.sThumbLY = (SHORT)(joyY * -32767.0f); 

	vigem_target_x360_update(g_client, g_pad, g_report);
}

void HandleUpDown(Button* button, bool down)
{
	switch (button->type)
	{
	case BTN_KEY:
		// Route through our new Xbox Button handler!
		HandleXboxButton(button, down);
		break;
	case BTN_WHEEL:
		HandleWheelButton(button, down);
		break;
	case BTN_QUIT:
		HandleQuitButton(button, down);
		break;
	}
}

LRESULT CALLBACK OnTouch(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BUTTON(hWnd, button);
	TOUCHINPUT touch;

	if (GetTouchInputInfo((HTOUCHINPUT)lParam, 1, &touch, sizeof(TOUCHINPUT)))
	{
		if (button->type == BTN_STICK)
		{
			TouchEvent event;
			if (touch.dwFlags & TOUCHEVENTF_DOWN)
			{
				event = TOUCH_DOWN;
			}
			else if (touch.dwFlags & TOUCHEVENTF_UP)
			{
				event = TOUCH_UP;
			}
			else
			{
				event = TOUCH_MOVE;
			}

			int clientX = touch.x / 100 - GetButtonX(button);
			int clientY = touch.y / 100 - GetButtonY(button);
			HandleStickButton(button, event, clientX, clientY);
		}
		else if (touch.dwFlags & (TOUCHEVENTF_DOWN | TOUCHEVENTF_UP))
		{
			HandleUpDown(button, touch.dwFlags & TOUCHEVENTF_DOWN);
		}

		CloseTouchInputHandle((HTOUCHINPUT)lParam);
		return 0;
	}
	else
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

bool IsFakeMouseEvent()
{
	return (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH;
}

LRESULT CALLBACK OnMouseButton(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(wParam);

	if (IsFakeMouseEvent()) { return 0; }

	BUTTON(hWnd, button);
	if (button->type == BTN_STICK)
	{
		TouchEvent event = uMsg == WM_LBUTTONDOWN ? TOUCH_DOWN : TOUCH_UP;
		HandleStickButton(
			button, event, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)
		);
	}
	else
	{
		HandleUpDown(button, uMsg == WM_LBUTTONDOWN);
	}

	return 0;
}

LRESULT CALLBACK OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNUSED(uMsg);

	if (IsFakeMouseEvent()) { return 0; }

	BUTTON(hWnd, button);
	if ((button->type == BTN_STICK) && (wParam & MK_LBUTTON))
	{
		HandleStickButton(
			button, TOUCH_MOVE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)
		);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		SetWindowLongPtr(
			hWnd,
			GWLP_USERDATA,
			(LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)
		);
		return 0;
	case WM_PAINT:
		return Paint(hWnd, uMsg, wParam, lParam);
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_TOUCH:
		return OnTouch(hWnd, uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		return OnMouseButton(hWnd, uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void RegisterGamepadWindowClass()
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = &WindowProc;
	wc.lpszClassName = "TouchJoy";
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_HAND);
	RegisterClass(&wc);
}

void InitializeGamepad(Gamepad* gamepad)
{
	for (int i = 0; i < gamepad->numButtons; ++i)
	{
		Button* button = &gamepad->buttons[i];

		HWND hwnd = CreateWindowEx(
			WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE, // Ex styles
			"TouchJoy", // Class name
			button->name, // Title
			WS_VISIBLE | WS_POPUP, // Styles
			GetButtonX(button), GetButtonY(button), // Position
			button->width, button->height, // Size
			NULL, // Parent
			NULL, // Menu
			GetModuleHandle(NULL),
			button // Extra param
		);
		ShowWindow(hwnd, SW_SHOW);
		SetLayeredWindowAttributes(
			hwnd, button->colorKey, 180, LWA_ALPHA | LWA_COLORKEY
		);
		RegisterTouchWindow(hwnd, TWF_FINETOUCH | TWF_WANTPALM);

		button->window = hwnd;
	}
}

void DeinitializeGamepad(Gamepad* gamepad)
{
	for (int i = 0; i < gamepad->numButtons; ++i)
	{
		Button* button = &gamepad->buttons[i];

		if (button->window)
		{
			DestroyWindow(button->window);
			button->window = 0;
		}
	}
}