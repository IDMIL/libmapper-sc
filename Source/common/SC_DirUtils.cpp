/*
 *  Created by Tim Walters on 10/19/05. 
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef SC_WIN32
# include <direct.h>
# include "SC_Win32Utils.h"
#else
# include <dirent.h>
# include <glob.h>
# include <sys/param.h>
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include <stdexcept>
#include "SC_DirUtils.h"

#ifdef SC_DARWIN
#ifndef _SC_StandAloneInfo_
# include "SC_StandAloneInfo_Darwin.h"
#endif
# include <CoreFoundation/CFString.h>
# include <CoreFoundation/CFBundle.h>
# include <CoreServices/CoreServices.h>
#endif

// Add a component to a path.

void sc_AppendToPath(char *path, const char *component)
{
#if defined(SC_WIN32)
	strncat(path, "\\", PATH_MAX);
#else
	strncat(path, "/", PATH_MAX);
#endif
	strncat(path, component, PATH_MAX);
}

char *sc_StandardizePath(const char *path, char *newpath2) {
	char newpath1[MAXPATHLEN];

	newpath1[0] = '\0';
	newpath2[0] = '\0';

	size_t pathLen = strlen(path);

	if ((pathLen >= 2) && (path[0] == '~') && ((path[1] == '/') || (path[1] == '\\'))) {
      char home[PATH_MAX];
      sc_GetUserHomeDirectory(home, PATH_MAX);
 
	    if (home != 0) {
			if ((pathLen - 1 + strlen(home)) >= MAXPATHLEN) {
				return 0;
			}
			strcpy(newpath1, home);
			strcat(newpath1, path + 1);
		} else {
			if (pathLen >= MAXPATHLEN) {
				return 0;
			}
			strcpy(newpath1, path);
			newpath1[0] = '.';
		}
	} else {
		if (pathLen >= MAXPATHLEN) {
			return 0;
		}
		strcpy(newpath1, path);
	}
  
	bool isAlias = false;
	sc_ResolveIfAlias(newpath1, newpath2, isAlias, PATH_MAX);

	return newpath2;
}


// Returns TRUE iff dirname is an existing directory.

bool sc_DirectoryExists(const char *dirname) 
{
#if defined(SC_WIN32)
	DWORD attr = GetFileAttributes(dirname);
	return ((attr != INVALID_FILE_ATTRIBUTES) &&
			(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat buf;
	return ((stat(dirname, &buf) == 0) &&
			S_ISDIR(buf.st_mode));
#endif
}

bool sc_IsSymlink(const char* path)
{
#if defined(SC_WIN32)
	// FIXME
	return false;
#else
	struct stat buf;
	return ((stat(path, &buf) == 0) &&
			S_ISLNK(buf.st_mode));
#endif
}

bool sc_IsNonHostPlatformDir(const char *name)
{
#if defined(SC_DARWIN)
	char a[] = "linux", b[] = "windows";
#elif defined(SC_LINUX)
	char a[] = "osx", b[] = "windows";
#elif defined(SC_WIN32)
	char a[] = "osx", b[] = "linux";
#endif
	return ((strcmp(name, a) == 0) ||
			(strcmp(name, b) == 0));
}


// Returns TRUE iff 'name' is special directory '.' or '..'

inline static bool sc_IsSpecialDirectory(const char* name)
{
	return (strcmp(name, ".") == 0) || (strcmp(name, "..")  == 0);
}

// Returns TRUE iff 'name' is to be ignored during compilation.

bool sc_SkipDirectory(const char *name)
{
	return ((strcasecmp(name, "help") == 0) ||
			(strcasecmp(name, "test") == 0) ||
			(strcasecmp(name, "_darcs") == 0) ||
			sc_IsNonHostPlatformDir(name));
}


void sc_ResolveIfAlias(const char *path, char *returnPath, bool &isAlias, int length) 
{
	isAlias = false;
#if defined(SC_DARWIN)
	FSRef dirRef;
	OSStatus osStatusErr = FSPathMakeRef ((const UInt8 *) path, &dirRef, NULL);
	if ( !osStatusErr ) {
		Boolean isFolder;
		Boolean wasAliased;
		OSErr err = FSResolveAliasFile (&dirRef, true, &isFolder, &wasAliased);
		isAlias = wasAliased;
		if ( !err && wasAliased ) {
			UInt8 resolvedPath[PATH_MAX];
			osStatusErr = FSRefMakePath (&dirRef, resolvedPath, length);
			if ( !osStatusErr ) {
				strncpy(returnPath, (char *) resolvedPath, length);
				return;
			}
		}
	}
#elif defined(SC_LINUX)
	isAlias = sc_IsSymlink(path);
	if (!realpath(path, returnPath))
		strcpy(returnPath, path);
	return;
#endif
	strcpy(returnPath, path);
	return;
}

// Support for Bundles

#ifndef SC_DARWIN

bool sc_IsStandAlone()
{
	return false;
}

void sc_GetResourceDirectory(char* pathBuf, int length)
{
	sc_GetResourceDirectoryFromAppDirectory(pathBuf, length);
}

#else	// running on OS X

// Support for stand-alone applications

bool sc_IsStandAlone()
{
	return SC_StandAloneInfo::IsStandAlone();
}

void sc_GetResourceDirectory(char* pathBuf, int length)
{
	SC_StandAloneInfo::GetResourceDir(pathBuf, length);
}

#endif	// ifndef SC_DARWIN

void sc_GetResourceDirectoryFromAppDirectory(char* pathBuf, int length)
{
	getcwd(pathBuf, length);
}


// Support for Extensions

// Get the user home directory.

void sc_GetUserHomeDirectory(char *str, int size)
{
#ifndef SC_WIN32
	char *home = getenv("HOME");
	strncpy(str, home, size);
#else
	win32_GetHomeFolder(str,size);
#endif
}


// Get the System level data directory.

void sc_GetSystemAppSupportDirectory(char *str, int size)
{
	strncpy(str,
#if defined(SC_DATA_DIR)
			SC_DATA_DIR,
#elif defined(SC_DARWIN)
			"/Library/Application Support/SuperCollider",
#elif defined(SC_WIN32)
			"C:\\SuperCollider",
#else
			"/usr/local/share/SuperCollider",
#endif
			size);
}


// Get the User level data directory.

void sc_GetUserAppSupportDirectory(char *str, int size)
{
	char home[PATH_MAX];
	sc_GetUserHomeDirectory(home, PATH_MAX);
	
	snprintf(str,
			 size,
#ifdef SC_DARWIN
			 "%s/Library/Application Support/SuperCollider",
#elif defined(SC_WIN32)
			"%s\\SuperCollider",
#else
			 "%s/share/SuperCollider",
#endif
			 home);
}


// Get the System level 'Extensions' directory.

void sc_GetSystemExtensionDirectory(char *str, int size)
{
	char path[PATH_MAX];
	sc_GetSystemAppSupportDirectory(path, sizeof(path));
	sc_AppendToPath(path, "Extensions");
	strncpy(str, path, size);
}


// Get the System level 'Extensions' directory.

void sc_GetUserExtensionDirectory(char *str, int size)
{
	char path[PATH_MAX];
	sc_GetUserAppSupportDirectory(path, sizeof(path));
	sc_AppendToPath(path, "Extensions");
	strncpy(str, path, size);
}


// Directory access

struct SC_DirHandle
{
#ifdef SC_WIN32
	HANDLE mHandle;
	WIN32_FIND_DATA mEntry;
	bool mAtEnd;
#else
	DIR* mHandle;
	struct dirent* mEntry;
#endif
};

SC_DirHandle* sc_OpenDir(const char* dirname)
{
	SC_DirHandle* dir = new SC_DirHandle;
	memset(dir, 0, sizeof(SC_DirHandle));

#ifdef SC_WIN32
	char allInDir[PATH_MAX];
	snprintf(allInDir, PATH_MAX, "%s\\*.*", dirname);

	dir->mHandle = ::FindFirstFile(allInDir, &dir->mEntry);
	if (dir->mHandle == INVALID_HANDLE_VALUE) {
		delete dir;
		return 0;
	}

	dir->mAtEnd = false;
#else
	dir->mHandle = opendir(dirname);	
	if (!dir->mHandle) {
		delete dir;
		return 0;
	}
#endif

	return dir;
}

void sc_CloseDir(SC_DirHandle* dir)
{
#ifdef SC_WIN32
	::FindClose(dir->mHandle);
#else
	closedir(dir->mHandle);
#endif
	delete dir;
}

bool sc_ReadDir(SC_DirHandle* dir, char* dirname, char* path, bool& skipEntry)
{
#ifdef SC_WIN32
	bool success = true;

	if (dir->mAtEnd)
		return false;

	const char* entry = dir->mEntry.cFileName;

	if (sc_IsSpecialDirectory(entry) || (skipEntry && sc_SkipDirectory(entry))) {
		skipEntry = true;
		success = true;
	} else {
		skipEntry = false;
		success = true;
	}

    char entrypathname[PATH_MAX];
	strncpy(entrypathname, dirname, PATH_MAX);
	sc_AppendToPath(entrypathname, dir->mEntry.cFileName);

	bool isAlias = false;
	sc_ResolveIfAlias(entrypathname, path, isAlias, PATH_MAX);

	if (!::FindNextFile(dir->mHandle, &dir->mEntry)) {
		dir->mAtEnd = true;
	}

	return true;
#else
	if (!dir->mHandle)
		return false;

	dir->mEntry = readdir(dir->mHandle);
	if (!dir->mEntry)
		return false;

	const char* entry = dir->mEntry->d_name;

	if (sc_IsSpecialDirectory(entry) || (skipEntry && sc_SkipDirectory(entry))) {
		skipEntry = true;
		return true;
	} else {
		skipEntry = false;
	}

	// construct path from dir entry
	char entrypathname[PATH_MAX];
	strncpy(entrypathname, dirname, PATH_MAX);
	sc_AppendToPath(entrypathname, dir->mEntry->d_name);

	// resolve path
	bool isAlias = false;
	sc_ResolveIfAlias(entrypathname, path, isAlias, PATH_MAX);

	return true;
#endif
}


// Globbing

struct SC_GlobHandle
{
#ifdef SC_WIN32
	HANDLE mHandle;
	char mFolder[PATH_MAX];
	WIN32_FIND_DATA mEntry;
	char mEntryPath[PATH_MAX];
	bool mAtEnd;
#else
	glob_t mHandle;
	size_t mEntry;
#endif
};

SC_GlobHandle* sc_Glob(const char* pattern)
{
	SC_GlobHandle* glob = new SC_GlobHandle;

#ifdef SC_WIN32
	char patternWin[1024];

	strncpy(patternWin, pattern, 1024);
	patternWin[1023] = 0;
	win32_ReplaceCharInString(patternWin, 1024, '/', '\\');

	win32_ExtractContainingFolder(glob->mFolder, patternWin, PATH_MAX);

	glob->mHandle = ::FindFirstFile(patternWin, &glob->mEntry);
	if (glob->mHandle == INVALID_HANDLE_VALUE) {
		delete glob;
		return 0;
	}

	glob->mAtEnd = false;
#else
	int flags = GLOB_MARK | GLOB_TILDE;
#ifdef SC_DARWIN
	flags |= GLOB_QUOTE;
#endif

	int err = ::glob(pattern, flags, NULL, &glob->mHandle);
	if (err < 0) {
		delete glob;
		return 0;
	}

	glob->mEntry = 0;
#endif

	return glob;
}

void sc_GlobFree(SC_GlobHandle* glob)
{
#ifdef SC_WIN32
	::FindClose(glob->mHandle);
#else
	globfree(&glob->mHandle);
#endif
	delete glob;
}

const char* sc_GlobNext(SC_GlobHandle* glob)
{
#ifdef SC_WIN32
	if (glob->mAtEnd)
		return 0;
	strncpy(glob->mEntryPath, glob->mFolder, PATH_MAX);
	sc_AppendToPath(glob->mEntryPath, glob->mEntry.cFileName);
	if (!::FindNextFile(glob->mHandle, &glob->mEntry))
		glob->mAtEnd = true;
	return glob->mEntryPath;
#else
	if (glob->mEntry >= glob->mHandle.gl_pathc)
		return 0;
	return glob->mHandle.gl_pathv[glob->mEntry++];
#endif
}