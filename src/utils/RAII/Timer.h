#pragma once
#include <windows.h>

// Window timer wrapper
class TimerWrapper {
private:
	HWND hwnd_;
	UINT_PTR timerId_;
	bool isActive_;

public:
	TimerWrapper(HWND hwnd, UINT_PTR timerId)
		: hwnd_(hwnd), timerId_(timerId), isActive_(false) {
	}

	~TimerWrapper() {
		Kill();
	}

	// Non-copyable
	TimerWrapper(const TimerWrapper&) = delete;
	TimerWrapper& operator=(const TimerWrapper&) = delete;

	// Movable
	TimerWrapper(TimerWrapper&& other) noexcept
		: hwnd_(other.hwnd_), timerId_(other.timerId_), isActive_(other.isActive_) {
		other.hwnd_ = nullptr;
		other.timerId_ = 0;
		other.isActive_ = false;
	}

	TimerWrapper& operator=(TimerWrapper&& other) noexcept {
		if (this != &other) {
			Kill();
			hwnd_ = other.hwnd_;
			timerId_ = other.timerId_;
			isActive_ = other.isActive_;
			other.hwnd_ = nullptr;
			other.timerId_ = 0;
			other.isActive_ = false;
		}
		return *this;
	}

	// Timer management
	bool Set(UINT delay) {
		if (!hwnd_ || !timerId_) return false;
		Kill(); // Kill existing timer first
		isActive_ = ::SetTimer(hwnd_, timerId_, delay, nullptr) != 0;
		return isActive_;
	}

	bool Kill() {
		if (!hwnd_ || !timerId_ || !isActive_) return true;
		bool result = ::KillTimer(hwnd_, timerId_) != 0;
		isActive_ = false;
		return result;
	}

	bool Restart(UINT delay) {
		return Set(delay);
	}

	bool IsActive() const { return isActive_; }
	UINT_PTR getId() const { return timerId_; }
	HWND getWindow() const { return hwnd_; }
};
