#pragma once
#include "data.hpp"
#include "util.hpp"
#include "httplib.h"


namespace Cloud
{
#define server_ip "123.60.163.45"
#define server_port 8888
	class Backup
	{
	private:
		std::string _back_dir;
		DataManager* data;
		
		std::pair<std::string, std::string> cookie;
	public:

		Backup(const std::string& back_dir, const std::string& back_file) :
			_back_dir(back_dir) {
			data = new DataManager(back_file);
			setcookie(cookie);

			verify(cookie);
		}

		bool verify(const std::pair<std::string, std::string>& cookie)
		{
			// ���� HTTP �ͻ���
			httplib::Client client(server_ip, server_port);

			// ��¼������
			std::string username = cookie.first; // �滻Ϊʵ���û���
			std::string password = cookie.second;  // �滻Ϊʵ������

			std::string form_data = "username=" + username + "&password=" + password;

			auto res = client.Post("/Login", form_data, "application/x-www-form-urlencoded");

			if (res && res->status == 200) {
				std::cout << "Login succeeded: " << res->body << std::endl;
				return true;
			}
			else {
				std::cout << "Login failed" << std::endl;
				if (res) {
					std::cout << "HTTP Status: " << res->status << std::endl;
				}
				else {
					std::cout << "Connection failed" << std::endl;
				}

				return false;
			}

			return true;
		}

		bool setcookie(std::pair<std::string, std::string>& cookie)
		{
			std::cin >> cookie.first;
			std::cin >> cookie.second;

			return true;
		}

		bool Upload(const std::string filename, const std::pair<std::string, std::string>& cookie)
		{
			FileUtil fu(filename);
			std::string body;
			fu.GetContent(&body);

			//�����������ӣ��ϴ��ļ�
			httplib::Client client(server_ip, server_port);
			// ���� Cookie
			httplib::MultipartFormData item;
			item.content = body;
			item.filename = fu.FileName();
			item.name = "file";
			item.content_type = "application/octet-stream";
			httplib::MultipartFormDataItems items;
			items.push_back(item);

			httplib::Headers headers = {
				{"Cookie", "username=" + cookie.first + "; Password=" + cookie.second}
			};

			auto res = client.Post("/Upload", headers, items);
			if (!res || res->status != 200) {
				return false;
			}
			return true;

		}


		std::string GetFileId(const std::string filename)
		{
			/*FileUtil fu(filename);
			std::string id = fu.FileName();
			id += " ";
			id += fu.FileName() + "-" + std::to_string(fu.FileSize()) + "-" + std::to_string(fu.LastATime());

			return id;*/

			FileUtil fu(filename);
			std::stringstream ss;
			ss << fu.FileName() << "-" << fu.FileSize() << "-" << fu.LastMTime();
			return ss.str();
		}
		


		bool IsNeedUpload(const std::string filename)
		{

			std::string id;   //Ψһ��־��
			if (data->GetOneByKey(filename, &id) != false)
			{
				//����new_id
				std::string new_id = GetFileId(filename);

				//std::cout << new_id << std::endl;
				if (new_id == id)
				{
					return false;   //�ļ��ϴ���Żᱸ���ļ���Ϣ��û�иı�Ͳ����ٴ��ϴ�
				}

			}

			//û���ҵ�������Ϣ���������ϴ��������޸���

			FileUtil fu(filename);
			if (time(NULL) - fu.LastATime() < 3)
			{
				// ���һ�η���ʱ��Ͻ�
				return false;  
			}

			return true; //��������
		}

		bool Run_module()
		{
			while (1)
			{
				FileUtil fu(_back_dir);
				std::vector<std::string> files;
				fu.DirectoryFile(files);

				for (const auto& e : files)
				{
					if (IsNeedUpload(e) == false)
					{
						continue;
					}
					// �����ļ���Ϣ
					// �ϴ��ļ�
					if (Upload(e,cookie) == true)
					{
						data->Insert(e, GetFileId(e));
						std::cout << "file backup success" << e << std::endl;
					}
				}

				Sleep(1000);
				std::cout << "-------------------------loop end------------\n";
			}

			return true;

		}
	};
}