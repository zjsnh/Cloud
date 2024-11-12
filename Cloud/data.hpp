#pragma once
#include "util.hpp"
#include "unordered_map"
#include <sstream>
 
namespace Cloud
{
	class DataManager
	{
	private:
		std::unordered_map<std::string, std::string> _table;
		std::string _backup_file;

	public:
		DataManager(std::string backup_file)
			:_backup_file(backup_file)
		{
			InitLoad();
		}

		bool Stoarge()
		{
			std::stringstream ss;
			auto it = _table.begin();
			while (it != _table.end())
			{
				ss << it->first << " " << it->second << "\n";
				it++;
			}

			FileUtil fu(_backup_file);
			fu.SetContent(ss.str());

			return true;
		}

		int Split(const std::string& str, const std::string& sep, std::vector<std::string>* array)
		{
			int count = 0;
			size_t pos = 0, idx = 0;

			while (1)
			{
				pos = str.find(sep,idx);
				if (pos == std::string::npos)
					break;

				if (pos == idx)
				{
					idx = pos + sep.size();
					continue;
				}

				std::string tmp = str.substr(idx, pos - idx);
				array->push_back(tmp);
				count++;
				idx = pos + sep.size();
			}
			if (idx < str.size())
			{
				//后面还有数据
				array->push_back(str.substr(idx));
				count++;
			}

			return count;

		}

		bool InitLoad()
		{
			FileUtil fu(_backup_file);
			std::string body;
			fu.GetContent(&body);

			std::vector<std::string> array;
			Split(body, "\n", &array);

			for (const auto& e: array)
			{
				std::vector<std::string> tmp;
				if (Split(e, " ", &tmp) != 2)
					continue;

				_table[tmp[0]] = tmp[1];
			}

			return true;
 
		}

		// filename filename-fsize-mtime
		bool Insert(const std::string& key, const std::string& value)
		{
			_table[key] = value;
			Stoarge();
			return true;
		}

		bool Updata(const std::string& key, const std::string& value)
		{
			_table[key] = value;
			Stoarge();
			return true;
		}

		bool GetOneByKey(const std::string& key, std::string* value)
		{
			auto it = _table.find(key);
			if (it == _table.end())
			{
				return false;
			}
			*value = it->second;
			return true;

		}
	};
}