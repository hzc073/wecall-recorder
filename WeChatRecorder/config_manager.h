/*
 * =====================================================================================
 *
 * Filename:  config_manager.h
 *
 * Description:  Declares functions for saving and loading application settings.
 *
 * =====================================================================================
 */
#pragma once

#include <windows.h>
#include <string>
#include <vector>

 // Loads all settings from the config.ini file.
 // Should be called on application startup.
void LoadConfig();

// Saves all current settings to the config.ini file.
// Should be called whenever a setting is changed or on exit.
void SaveConfig();
#pragma once
