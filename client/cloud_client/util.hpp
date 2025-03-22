#ifndef __MY_UTIL__
#define __MY_UTIL__
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <experimental/filesystem>
#include <sys/stat.h>

namespace cloud{
	namespace fs = std::experimental::filesystem;
	class FileUtil{
		private:
			std::string _filename;
		public:
			FileUtil(const std::string &filename):_filename(filename){}
			bool Remove(){
				if (this->Exists()== false) {
					return true;
				}
				remove(_filename.c_str());
				return true;
			}
			size_t FileSize(){
				struct stat st;
				if (stat(_filename.c_str(), &st) < 0) {
					std::cout << "get file size failed!\n";
					return 0;
				}
				return st.st_size;
			}
			time_t LastMTime(){
				struct stat st;
				if (stat(_filename.c_str(), &st) < 0) {
					std::cout << "get file size failed!\n";
					return -1;
				}
				return st.st_mtime;
			}
			time_t LastATime() {
				struct stat st;
				if (stat(_filename.c_str(), &st) < 0) {
					std::cout << "get file size failed!\n";
					return -1;
				}
				return st.st_atime;
			}
			std::string FileName(){
				// ./abc/test.txt
				size_t pos = _filename.find_last_of("\\");
				if (pos == std::string::npos) {
					return _filename;
				}
				// return fs::path(_filename).filename().string();
				return _filename.substr(pos+1);
			}
			bool GetPosLen(std::string *body, size_t pos, size_t len){
				size_t fsize = this->FileSize();
				if (pos + len > fsize){
					std::cout << "get file len is error\n";
					return false;
				}
				std::ifstream ifs;
				ifs.open(_filename, std::ios::binary);
				if (ifs.is_open() == false) {
					std::cout << "read open file failed!\n";
					return false;
				}
				ifs.seekg(pos, std::ios::beg);
				body->resize(len);
				ifs.read(&(*body)[0], len);
				if (ifs.good() == false) {
					std::cout << "get file content failed\n";
					ifs.close();
					return false;
				}
				ifs.close();
				return true;
			}
			bool GetContent(std::string *body) {
				size_t fsize = this->FileSize();
				return GetPosLen(body, 0, fsize);
			}
			bool SetContent(const std::string &body) {
				std::ofstream ofs;
				ofs.open(_filename, std::ios::binary);
				if (ofs.is_open() == false) {
					std::cout << "write open file failed!\n";
					return false;
				}
				ofs.write(&body[0], body.size());
				if (ofs.good() == false) {
					std::cout << "write file content failed!\n";
					ofs.close();
					return false;
				}
				ofs.close();
				return true;
			}
			bool Exists() {
				return fs::exists(_filename);
			}
			bool CreateDirectory() {
				if (this->Exists()) return true;
				return fs::create_directories(_filename);
			}
			bool ScanDirectory(std::vector<std::string> *arry) {
				this->CreateDirectory();
				for(auto& p: fs::directory_iterator(_filename)) {
					if (fs::is_directory(p) == true){
						continue;
					}
					//relative_path 带有路径的文件名
					arry->push_back(fs::path(p).relative_path().string());
				}
				return true;
			}
	};
}
#endif
