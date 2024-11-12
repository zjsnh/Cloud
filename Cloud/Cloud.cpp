#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 1
#define _CRT_SECURE_NO_WARNINGS 1
#include "util.hpp"
#include "Cloud.hpp"

#define BACKUP_DIR "../synchronous"
#define BACKUP_FILE "./backup.dat"

int main()
{
	/*Cloud::FileUtil fu("./");
	std::vector<std::string> files;

	fu.DirectoryFile(files);

	for (const auto& e : files)
	{
		std::cout << e << std::endl;
	}*/

	Cloud::Backup bu(BACKUP_DIR, BACKUP_FILE);
	bu.Run_module();

	return 0;
}