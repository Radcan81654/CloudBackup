#ifndef __MY_CLOUD__
#define __MY_CLOUD__
#include "data.hpp"
#include "httplib.h"
#include <windows.h>

namespace cloud {
#define SERVER_ADDR "123.56.166.61"
#define SERVER_PORT 9090
	class Backup {
	private:
		std::string _back_dir;
		DataManager *_data;
	public:
		Backup(const std::string &back_dir, const std::string &back_file):
			_back_dir(back_dir) {
			_data = new DataManager(back_file);
		}
		std::string GetFileIdentifier(const std::string &filename) {
			// a.txt-fsize-mtime
			FileUtil fu(filename);
			std::stringstream ss;
			ss << fu.FileName() << "-" << fu.FileSize() << "-" << fu.LastMTime();
			return ss.str();
		}
		bool Upload(const std::string &filename) {
			//1. 获取文件数据
			FileUtil fu(filename);
			std::string body;
			fu.GetContent(&body);

			//2. 搭建http客户端上传文件数据
			httplib::Client client(SERVER_ADDR, SERVER_PORT);
			httplib::MultipartFormData item;
			item.content = body;
			item.filename = fu.FileName();
			item.name = "file";
			item.content_type = "application/octet-stream";
			httplib::MultipartFormDataItems items;
			items.push_back(item);

			auto res = client.Post("/upload", items);
			if (!res || res->status != 200) {
				return false;
			}
			return true;
		}
		bool IsNeedUpload(const std::string &filename) {
			// 需要上传的文件的判断条件：文件是新增的，不是新增但是被修改过
			// 文件是新增的：看一下有没有历史备份信息
			// 不是新增但是被修改过：有历史信息，但是历史的唯一标识与当前最新的唯一标识不一致
			std::string id;
			if (_data->GetOneByKey(filename, &id) != false) {
				//有历史信息
				std::string new_id = GetFileIdentifier(filename);
				if (new_id == id) {
					return false;//不需要被上传-上次上传后没有被修改过
				}
			}
			//一个文件比较大,正在徐徐的拷贝到这个目录下，需要过程，
			//因此判断一个文件一段时间都没有被修改过了，才上传
			FileUtil fu(filename);
			if (time(NULL) - fu.LastMTime() < 3) {//3秒钟之内刚修改过--认为文件还在修改中
				return false;
			}

			std::cout << filename << " need upload!\n";
			return true;
		}
		bool RunModule() {
			while(1) {
				//1. 遍历获取指定文件夹中所有文件
				FileUtil fu(_back_dir);
				std::vector<std::string> arry;
				fu.ScanDirectory(&arry);
				//2. 逐个判断文件是否需要上传
				for (auto &a : arry) {
					if (IsNeedUpload(a) == false) {
						continue;
					}
					//3. 如果需需要上传则上传文件
					if (Upload(a) == true) {
						_data->Insert(a, GetFileIdentifier(a));//新增文件备份信息
						std::cout << a << " upload success!\n";
					}
				}
				Sleep(1);
				std::cout << "-------------------------loop end------------\n";
			}
		}

	};
}

#endif
