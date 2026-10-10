// System services on SDL3: see port/sys.h.

#include "platform/platform.h"
#include "platform/testing.h"
#include "port/sys.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <map>
#include <string>
#include <vector>

namespace {

SDL_Window *s_window;
std::string s_dataDir;     // game data, read only (ends with '/')
std::string s_userDir;     // saves, settings, logs (ends with '/')
std::string s_commandLine;
std::map<std::string, std::string> s_settings;  // "section.key" -> value
FILE *s_logFile;
const PlatformTestClock *s_testClock;

// ---- paths -----------------------------------------------------------------

std::string Lower(const std::string &s)
{
    std::string out = s;
    for (char &c : out)
        c = (char)SDL_tolower((unsigned char)c);
    return out;
}

// Splits a game path into its components: drive letters, "." and empty
// components dropped, ".." applied.
std::vector<std::string> SplitGamePath(const char *path)
{
    std::vector<std::string> parts;
    const char *p = path;

    if (p[0] && p[1] == ':')
        p += 2;
    std::string part;
    for (;; p++) {
        if (*p == '\\' || *p == '/' || *p == 0) {
            if (part == "..") {
                if (!parts.empty())
                    parts.pop_back();
            } else if (!part.empty() && part != ".") {
                parts.push_back(part);
            }
            part.clear();
            if (*p == 0)
                break;
        } else {
            part += *p;
        }
    }
    return parts;
}

// Directory listings, lower-case name -> name on disk.
std::map<std::string, std::map<std::string, std::string>> s_dirCache;

const std::map<std::string, std::string> &ListDir(const std::string &dir)
{
    auto it = s_dirCache.find(dir);
    if (it != s_dirCache.end())
        return it->second;

    std::map<std::string, std::string> &entries = s_dirCache[dir];
    int count = 0;
    char **names = SDL_GlobDirectory(dir.c_str(), NULL, 0, &count);
    if (names != NULL) {
        for (int i = 0; i < count; i++) {
            std::string name = names[i];
            if (!name.empty() && name.back() == '/')
                name.pop_back();
            entries.emplace(Lower(name), name);
        }
        SDL_free(names);
    }
    return entries;
}

// Finds parts under root without regard to case; returns the host path or "".
std::string FindUnder(const std::string &root, const std::vector<std::string> &parts)
{
    std::string path = root;
    for (size_t i = 0; i < parts.size(); i++) {
        const auto &entries = ListDir(path);
        auto it = entries.find(Lower(parts[i]));
        if (it == entries.end())
            return std::string();
        path += it->second;
        if (i + 1 < parts.size())
            path += '/';
    }
    return path;
}

// Host path for writing under the user directory; creates the directories,
// reusing existing ones whatever their case.
std::string PathForWriting(const std::vector<std::string> &parts)
{
    std::string path = s_userDir;
    for (size_t i = 0; i < parts.size(); i++) {
        const auto &entries = ListDir(path);
        auto it = entries.find(Lower(parts[i]));
        std::string name = it != entries.end() ? it->second : parts[i];
        bool isDir = i + 1 < parts.size();
        if (it == entries.end()) {
            if (isDir)
                SDL_CreateDirectory((path + name).c_str());
            s_dirCache[path].emplace(Lower(name), name);
        }
        path += name;
        if (isDir)
            path += '/';
    }
    return path;
}

std::string ResolveForReading(const char *gamePath)
{
    std::vector<std::string> parts = SplitGamePath(gamePath);
    if (parts.empty())
        return std::string();
    std::string found = FindUnder(s_userDir, parts);
    if (found.empty())
        found = FindUnder(s_dataDir, parts);
    return found;
}

bool MatchPattern(const std::string &name, const std::string &pattern)
{
    // One '*' at most, compared without regard to case ("*.sav").
    std::string n = Lower(name), p = Lower(pattern);
    size_t star = p.find('*');
    if (star == std::string::npos)
        return n == p;
    std::string head = p.substr(0, star), tail = p.substr(star + 1);
    return n.size() >= head.size() + tail.size() && n.compare(0, head.size(), head) == 0 &&
           n.compare(n.size() - tail.size(), tail.size(), tail) == 0;
}

// ---- settings ----------------------------------------------------------------

void LoadSettings(const std::string &file)
{
    FILE *f = fopen(file.c_str(), "r");
    if (f == NULL)
        return;
    char line[512];
    std::string section;
    while (fgets(line, sizeof(line), f)) {
        std::string s = line;
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
            s.pop_back();
        size_t start = s.find_first_not_of(" \t");
        if (start == std::string::npos || s[start] == ';' || s[start] == '#')
            continue;
        s = s.substr(start);
        if (s[0] == '[') {
            section = Lower(s.substr(1, s.find(']') - 1));
            continue;
        }
        size_t eq = s.find('=');
        if (eq == std::string::npos)
            continue;
        std::string key = s.substr(0, eq), value = s.substr(eq + 1);
        while (!key.empty() && key.back() == ' ')
            key.pop_back();
        value.erase(0, value.find_first_not_of(' '));
        s_settings[section + "." + Lower(key)] = value;
    }
    fclose(f);
}

const char s_defaultSettings[] =
    "; OpenCMR2 settings\n"
    "\n"
    "[paths]\n"
    "; Directory of the original game data (Game, FrontEnd, sounds, ...).\n"
    "; data = /path/to/cmr2\n"
    "\n"
    "[install]\n"
    "; English, American, French, German, Italian, Spanish or Polish.\n"
    "language = English\n"
    "; EUROPE, AMERICA, POLAND or JAPAN.\n"
    "sku = EUROPE\n"
    "\n"
    "[video]\n"
    "; 1 = fullscreen (on the desktop resolution), 0 = window.\n"
    "fullscreen = 1\n";

bool LooksLikeGameData(const std::string &dir)
{
    return !FindUnder(dir, SplitGamePath("Game")).empty() && !FindUnder(dir, SplitGamePath("FrontEnd")).empty();
}

std::string WithSlash(std::string dir)
{
    if (!dir.empty() && dir.back() != '/')
        dir += '/';
    return dir;
}

} // namespace

// ---- platform lifecycle ------------------------------------------------------

bool Platform_Init(int argc, char **argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    char *pref = SDL_GetPrefPath("OpenCMR2", "OpenCMR2");
    s_userDir = pref ? pref : "./";
    SDL_free(pref);
    s_logFile = fopen((s_userDir + "opencmr2.log").c_str(), "w");

    std::string settingsFile = s_userDir + "opencmr2.ini";
    if (!SDL_GetPathInfo(settingsFile.c_str(), NULL)) {
        if (FILE *f = fopen(settingsFile.c_str(), "w")) {
            fputs(s_defaultSettings, f);
            fclose(f);
        }
    }
    LoadSettings(settingsFile);

    std::string dataDir;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            dataDir = argv[++i];
        } else {
            if (!s_commandLine.empty())
                s_commandLine += ' ';
            s_commandLine += argv[i];
        }
    }

    std::vector<std::string> candidates;
    if (!dataDir.empty())
        candidates.push_back(dataDir);
    if (const char *env = SDL_getenv("OPENCMR2_DATA"))
        candidates.push_back(env);
    if (const char *setting = Platform_GetSetting("paths.data", NULL))
        candidates.push_back(setting);
    candidates.push_back(".");
    if (const char *base = SDL_GetBasePath())
        candidates.push_back(base);

    for (const std::string &candidate : candidates) {
        std::string dir = WithSlash(candidate);
        if (LooksLikeGameData(dir)) {
            s_dataDir = dir;
            break;
        }
    }
    if (s_dataDir.empty()) {
        std::string message = "The Colin McRae Rally 2.0 game data was not found.\n\n"
                              "Pass its directory with --data <dir>, set OPENCMR2_DATA, or set\n"
                              "data = <dir> under [paths] in\n" + settingsFile;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenCMR2", message.c_str(), NULL);
        Sys_Log("%s", message.c_str());
        return false;
    }
    Sys_Log("game data: %s", s_dataDir.c_str());
    Sys_Log("user directory: %s", s_userDir.c_str());
    return true;
}

void Platform_Shutdown(void)
{
    Sys_DestroyWindow();
    if (s_logFile != NULL) {
        fclose(s_logFile);
        s_logFile = NULL;
    }
    SDL_Quit();
}

SDL_Window *Platform_GetWindow(void)
{
    return s_window;
}

const char *Platform_GetDataDir(void)
{
    return s_dataDir.c_str();
}

const char *Platform_GetUserDir(void)
{
    return s_userDir.c_str();
}

const char *Platform_GetSetting(const char *key, const char *fallback)
{
    auto it = s_settings.find(Lower(key));
    return it != s_settings.end() ? it->second.c_str() : fallback;
}

int Platform_GetSettingInt(const char *key, int fallback)
{
    const char *value = Platform_GetSetting(key, NULL);
    return value != NULL ? (int)strtol(value, NULL, 0) : fallback;
}

// ---- time ----------------------------------------------------------------------

extern "C" DWORD Sys_GetTicks(void)
{
    if (s_testClock)
        return s_testClock->ticks();
    return (DWORD)SDL_GetTicks();
}

extern "C" unsigned int Sys_GetUnixTime(void)
{
    return s_testClock ? s_testClock->unixTime : (unsigned int)time(NULL);
}

void Platform_SetTestClock(const PlatformTestClock *clock) { s_testClock = clock; }

extern "C" void Sys_Sleep(DWORD ms)
{
    if (s_testClock) {
        s_testClock->sleep(ms);
        return;
    }
    SDL_Delay(ms);
}

// ---- events ----------------------------------------------------------------------

namespace {

// Windows-1252 code of a Unicode code point, or 0.
int ToWindows1252(Uint32 cp)
{
    static const Uint16 high[32] = { 0x20ac, 0, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
                                     0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017d, 0,
                                     0, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
                                     0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0, 0x017e, 0x0178 };
    if (cp < 0x80 || (cp >= 0xa0 && cp <= 0xff))
        return (int)cp;
    for (int i = 0; i < 32; i++)
        if (high[i] == cp)
            return 0x80 + i;
    return 0;
}

// Text input can carry several characters; they are handed out one by one.
std::vector<int> s_pendingText;

bool Translate(const SDL_Event &e, SysEvent *out)
{
    memset(out, 0, sizeof(*out));
    switch (e.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        out->type = SYS_EVENT_QUIT;
        return true;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        out->type = SYS_EVENT_FOCUS_LOST;
        return true;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        out->type = SYS_EVENT_FOCUS_GAINED;
        return true;
    case SDL_EVENT_WINDOW_MINIMIZED:
        out->type = SYS_EVENT_MINIMIZED;
        return true;
    case SDL_EVENT_WINDOW_RESTORED:
        out->type = SYS_EVENT_RESTORED;
        return true;
    case SDL_EVENT_KEY_DOWN:
        out->type = SYS_EVENT_KEY_DOWN;
        out->key = Keys_FromScancode(e.key.scancode);
        out->repeat = e.key.repeat;
        return out->key != 0;
    case SDL_EVENT_TEXT_INPUT: {
        const char *text = e.text.text;
        Uint32 cp;
        while ((cp = SDL_StepUTF8(&text, NULL)) != 0) {
            int c = ToWindows1252(cp);
            if (c != 0)
                s_pendingText.push_back(c);
        }
        return false;
    }
    default:
        return false;
    }
}

bool TakePendingText(SysEvent *out)
{
    if (s_pendingText.empty())
        return false;
    memset(out, 0, sizeof(*out));
    out->type = SYS_EVENT_TEXT;
    out->character = s_pendingText.front();
    s_pendingText.erase(s_pendingText.begin());
    return true;
}

} // namespace

extern "C" int Sys_PollEvent(SysEvent *event)
{
    SDL_Event e;
    if (TakePendingText(event))
        return 1;
    while (SDL_PollEvent(&e)) {
        if (Translate(e, event))
            return 1;
        if (TakePendingText(event))
            return 1;
    }
    return 0;
}

extern "C" int Sys_WaitEvent(SysEvent *event)
{
    SDL_Event e;
    if (TakePendingText(event))
        return 1;
    while (SDL_WaitEvent(&e)) {
        if (Translate(e, event))
            return 1;
        if (TakePendingText(event))
            return 1;
    }
    return 0;
}

// ---- window ----------------------------------------------------------------------

extern "C" BOOL Sys_CreateWindow(const char *title, BOOL fullscreen)
{
    if (s_window != NULL)
        return TRUE;
    // The renderer sizes the window once it knows the resolution.
    SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    s_window = SDL_CreateWindow(title, 640, 480, flags);
    if (s_window == NULL) {
        Sys_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return FALSE;
    }
    SDL_StartTextInput(s_window);
    return TRUE;
}

extern "C" void Sys_DestroyWindow(void)
{
    if (s_window != NULL) {
        SDL_DestroyWindow(s_window);
        s_window = NULL;
    }
}

extern "C" void Sys_ShowCursor(BOOL show)
{
    if (show)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

extern "C" void Sys_GetDesktopSize(int *width, int *height)
{
    SDL_DisplayID display = s_window ? SDL_GetDisplayForWindow(s_window) : SDL_GetPrimaryDisplay();
    const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(display);
    *width = mode ? mode->w : 640;
    *height = mode ? mode->h : 480;
}

// ---- messages ----------------------------------------------------------------------

extern "C" int Sys_MessageBox(const char *title, const char *text, int buttons)
{
    SDL_MessageBoxButtonData okButtons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
          SYS_MESSAGEBOX_RESULT_OK, "OK" },
    };
    SDL_MessageBoxButtonData retryButtons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, SYS_MESSAGEBOX_RESULT_RETRY, "Retry" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, SYS_MESSAGEBOX_RESULT_CANCEL, "Cancel" },
    };
    SDL_MessageBoxData data = {};
    data.flags = SDL_MESSAGEBOX_ERROR;
    data.window = s_window;
    data.title = title;
    data.message = text;
    if (buttons == SYS_MESSAGEBOX_RETRY_CANCEL) {
        data.numbuttons = 2;
        data.buttons = retryButtons;
    } else {
        data.numbuttons = 1;
        data.buttons = okButtons;
    }
    int result = SYS_MESSAGEBOX_RESULT_CANCEL;
    if (!SDL_ShowMessageBox(&data, &result))
        Sys_Log("message box \"%s\": %s", title, text);
    return result;
}

extern "C" void Sys_Log(const char *format, ...)
{
    char line[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(line, sizeof(line), format, args);
    va_end(args);
    fprintf(stderr, "%s\n", line);
    if (s_logFile != NULL) {
        fprintf(s_logFile, "%s\n", line);
        fflush(s_logFile);
    }
}

extern "C" void Sys_Fatal(const char *format, ...)
{
    char line[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(line, sizeof(line), format, args);
    va_end(args);
    Sys_Log("fatal: %s", line);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenCMR2", line, s_window);
    exit(1);
}

extern "C" void Sys_Exit(int code)
{
    Platform_Shutdown();
    exit(code);
}

extern "C" const char *Sys_GetCommandLine(void)
{
    return s_commandLine.c_str();
}

// ---- files ----------------------------------------------------------------------

struct SysFile {
    FILE *f;
};

extern "C" BOOL Sys_ResolvePath(const char *path, BOOL forWriting, char *out, size_t outSize)
{
    std::string host;
    if (forWriting) {
        std::vector<std::string> parts = SplitGamePath(path);
        if (parts.empty())
            return FALSE;
        host = PathForWriting(parts);
    } else {
        host = ResolveForReading(path);
        if (host.empty())
            return FALSE;
    }
    if (host.size() + 1 > outSize)
        return FALSE;
    memcpy(out, host.c_str(), host.size() + 1);
    return TRUE;
}

extern "C" SysFile *Sys_OpenFile(const char *path, int mode)
{
    char host[1024];
    if (!Sys_ResolvePath(path, mode == SYS_FILE_WRITE, host, sizeof(host)))
        return NULL;
    FILE *f = fopen(host, mode == SYS_FILE_WRITE ? "wb" : "rb");
    if (f == NULL)
        return NULL;
    SysFile *file = new SysFile;
    file->f = f;
    return file;
}

extern "C" void Sys_CloseFile(SysFile *file)
{
    if (file != NULL) {
        fclose(file->f);
        delete file;
    }
}

extern "C" DWORD Sys_ReadFile(SysFile *file, void *buffer, DWORD size)
{
    return (DWORD)fread(buffer, 1, size, file->f);
}

extern "C" DWORD Sys_WriteFile(SysFile *file, const void *buffer, DWORD size)
{
    return (DWORD)fwrite(buffer, 1, size, file->f);
}

extern "C" DWORD Sys_SeekFile(SysFile *file, LONG offset, int origin)
{
    int whence = origin == SYS_SEEK_CUR ? SEEK_CUR : origin == SYS_SEEK_END ? SEEK_END : SEEK_SET;
    if (fseek(file->f, offset, whence) != 0)
        return 0xffffffff;
    return (DWORD)ftell(file->f);
}

extern "C" DWORD Sys_GetFileSize(SysFile *file)
{
    long position = ftell(file->f);
    if (position < 0 || fseek(file->f, 0, SEEK_END) != 0)
        return 0xffffffff;
    long size = ftell(file->f);
    fseek(file->f, position, SEEK_SET);
    return size < 0 ? 0xffffffff : (DWORD)size;
}

extern "C" void Sys_FlushFile(SysFile *file)
{
    fflush(file->f);
}

extern "C" BOOL Sys_FileExists(const char *path)
{
    return !ResolveForReading(path).empty();
}

extern "C" BOOL Sys_DeleteFile(const char *path)
{
    // Only files in the user directory can be deleted.
    std::string host = FindUnder(s_userDir, SplitGamePath(path));
    if (host.empty() || !SDL_RemovePath(host.c_str()))
        return FALSE;
    s_dirCache.clear();
    return TRUE;
}

extern "C" void Sys_ListFiles(const char *directory, const char *pattern, SysListCallback callback, void *context)
{
    std::vector<std::string> parts = SplitGamePath(directory);
    std::map<std::string, std::string> names;
    for (const std::string *root : { &s_dataDir, &s_userDir }) {
        std::string dir = parts.empty() ? *root : FindUnder(*root, parts);
        if (dir.empty())
            continue;
        dir = WithSlash(dir);
        for (const auto &entry : ListDir(dir)) {
            SDL_PathInfo info;
            if (MatchPattern(entry.second, pattern) && SDL_GetPathInfo((dir + entry.second).c_str(), &info) &&
                info.type == SDL_PATHTYPE_FILE)
                names[entry.first] = entry.second;   // the user directory's copy wins
        }
    }
    for (const auto &name : names)
        if (!callback(name.second.c_str(), context))
            break;
}

// ---- settings ----------------------------------------------------------------------

extern "C" const char *Sys_GetInstallSetting(const char *name)
{
    if (SDL_strcasecmp(name, "Game_HDPath") == 0 || SDL_strcasecmp(name, "Game_CDPath") == 0)
        return ".";
    if (SDL_strcasecmp(name, "Install_Version") == 0)
        return "Full";
    if (SDL_strcasecmp(name, "Language") == 0)
        return Platform_GetSetting("install.language", "English");
    if (SDL_strcasecmp(name, "Sku_Type") == 0)
        return Platform_GetSetting("install.sku", "EUROPE");
    return "";
}

extern "C" int Sys_GetOption(const char *name, int fallback)
{
    return Platform_GetSettingInt(name, fallback);
}
