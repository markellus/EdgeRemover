#include <iostream>
#include <filesystem>
#include <fstream>

#include "setupPatch.h"
#include "admin.h"

namespace fs = std::filesystem;

std::string findSetupExecutable()
{
  // We could also search the registry entry but i do not trust Microsoft. They will probably change it in the future.

  std::string strPath = R"(C:\Program Files (x86)\Microsoft\Edge\Application)";
  for (const auto & entry : fs::directory_iterator(strPath))
  {
    if(entry.path().string() != "SetupMetrics")
    {
      auto strProbe = entry.path().string().append("\\Installer\\setup.exe");

      if(fs::exists(strProbe))
      {
        return strProbe;
      }
    }
  }

  return "";
}

bool overrideSetup(const std::string& strExecutable)
{
  std::ofstream out(strExecutable, std::ios::binary | std::ios::trunc);

  if(!out.is_open())
  {
    return false;
  }
  out.write((char *)SETUP_PATCH,sizeof(SETUP_PATCH));
  out.close();
  return true;
}

bool uninstallEdge(const std::string& strExecutable)
{
  SHELLEXECUTEINFOA sei = {sizeof(sei)};
  sei.lpVerb       = "runas";
  sei.lpFile       = strExecutable.c_str();
  sei.hwnd         = nullptr;
  sei.lpParameters = "--uninstall --msedge --channel=stable --system-level --verbose-logging";
  sei.nShow        = SW_NORMAL;
  if (!ShellExecuteExA(&sei))
  {
    DWORD dwError = GetLastError();
    return false;
  }
  else
  {
    return true;
  }
}

int main()
{
  if (!elevateIfReqired())
  {
    std::cerr << "This tool requires administrative privileges in order to patch the Edge setup executable."
              << std::endl;
    return 3;
  }

  std::cout << "Windows 11 Edge Remover\n"
            << "Searching for Edge installation path..."
            << std::endl;

  std::string strExecutable = findSetupExecutable();

  if (strExecutable.empty())
  {
    std::cerr << "Unable to find Edge executable!" << std::endl;
    return 3;
  }

  std::cout << "Edge setup executable found at " << strExecutable << ", enabling override..." << std::endl;

  if (!overrideSetup(strExecutable))
  {
    std::cerr << "Unable to override edge setup file!"
              << std::endl;
    return 3;
  }

  std::cout << "Setup has been patched. The uninstallation process should now work again.\n"
            << "launching uninstall process..." << std::endl;

  if (!uninstallEdge(strExecutable))
  {
    std::cerr << "Unable to launch to uninstall process!"
              << std::endl;
    return 3;
  }

  return 0;
}