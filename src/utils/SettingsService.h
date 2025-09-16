#pragma once
#include <windows.h>

// Centralized settings access
class SettingsService {
public:
	// Click count
	static int ReadClickCount();
	static void WriteClickCount(int count);

	// Skin
	static int ReadSkin();
	static void WriteSkin(int skin);

	// Window position
	static bool ReadWindowPosition(int& x, int& y);
	static void WriteWindowPosition(int x, int y);

	// Startup
	static bool IsRunAtStartupEnabled();
	static bool SetRunAtStartup(bool enable);

	// First-run
	static bool IsFirstRun();
	static void MarkFirstRunCompleted();
};
