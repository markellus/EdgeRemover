#pragma once

#include <windows.h>

// https://www.codeproject.com/Articles/320748/Elevating-During-Runtime

bool isAppRunningAsAdminMode()
{
  BOOL                     bAdmin      = false;
  DWORD                    dwError     = ERROR_SUCCESS;
  PSID                     psidGroup   = nullptr;
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

  if (!AllocateAndInitializeSid(
    &NtAuthority,
    2,
    SECURITY_BUILTIN_DOMAIN_RID,
    DOMAIN_ALIAS_RID_ADMINS,
    0, 0, 0, 0, 0, 0,
    &psidGroup))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

  if (!CheckTokenMembership(nullptr, psidGroup, &bAdmin))
  {
    dwError = GetLastError();
    goto Cleanup;
  }

  Cleanup:
  // Centralized cleanup for all allocated resources.
  if (psidGroup)
  {
    FreeSid(psidGroup);
    psidGroup = NULL;
  }

  // Throw the error if something failed in the function.
  if (ERROR_SUCCESS != dwError)
  {
    throw dwError;
  }

  return bAdmin;
}

bool elevateIfReqired()
{
  if(isAppRunningAsAdminMode())
  {
    return true;
  }

  wchar_t szPath[MAX_PATH];
  if (GetModuleFileNameW(nullptr, szPath, ARRAYSIZE(szPath)))
  {
    // Launch itself as admin
    SHELLEXECUTEINFOW sei = {sizeof(sei)};
    sei.lpVerb = L"runas";
    sei.lpFile = szPath;
    sei.hwnd   = nullptr;
    sei.nShow  = SW_NORMAL;
    if (!ShellExecuteExW(&sei))
    {
      DWORD dwError = GetLastError();
      if (dwError == ERROR_CANCELLED)
      {
        return false;
      }
    }
    else
    {
      return true;
    }
  }

  return false;
}