#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "commondata.hpp"
namespace installation {
	void checkInstallFolder();
	void checkPreinstallFolder();
	void preinstallHeaders(const std::vector<headerFile>& headerToInstall);
	void preinstallBinaries();
	void installAll();
};
