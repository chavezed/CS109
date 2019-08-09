// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");

   this->root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   this->cwd = root;
   this->root->get_path() = "/";
   this->root->get_base()->get_dirents().insert(
            pair<string, inode_ptr>(".", root));
   this->root->get_base()->get_dirents().insert(
            pair<string, inode_ptr>("..", root));
   this->path.push_back("/");
   this->root->set_parent(this->root);
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   string temp;
   for (unsigned int i = 0; i < data.size(); ++i) {
      temp = data[i];
      size += temp.length();
   }
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   this->data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}


size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   
   size = dirents.size();
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   return nullptr;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   
   this->file_name = filename;
   shared_ptr<inode> new_file = 
                make_shared<inode>(file_type::PLAIN_TYPE);
   this->dirents.insert(pair<string, inode_ptr>(file_name, new_file));
   return new_file;
}

map<string,inode_ptr>& plain_file::get_dirents() {  }

map<string,inode_ptr>& directory::get_dirents() { return dirents; }

void plain_file::print_dirents() {}

void directory::print_dirents() {
   map<string, inode_ptr>::iterator entry;
   for (entry = dirents.begin(); entry != dirents.end(); ++entry) {
      if (entry->second->get_base()->get_type() == 
         file_type::DIRECTORY_TYPE
         and entry->first != "." and entry->first != ".." ) {
         cout << entry->second->get_inode_nr() << "   "    
              << entry->second->get_base()->size() << "   "
              << entry->first + "/" << endl;
      }
      else {
         cout << entry->second->get_inode_nr() << "   "    
              << entry->second->get_base()->size() << "   "
              << entry->first << endl;
      }
   }
}
