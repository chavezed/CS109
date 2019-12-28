// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $

#include "commands.h"
#include "debug.h"
#include <map>

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr curr_wd = state.get_cwd();

   if (words.size() == 1) {
      cerr << "cat: No file given\n";
      exit_status::set(1);
   }
   else {
      map<string, inode_ptr> cats = curr_wd->get_base()->get_dirents();
      map<string, inode_ptr>::iterator iter;
      wordvec toPrint; 
      for (unsigned int i = 1; i < words.size(); ++i) {
         iter = cats.find(words[i]);
         if (iter != cats.end() ) {
            toPrint = iter->second->get_base()->readfile();
            cout << toPrint << endl;
         } else if (iter == cats.end()) {
            cerr << "cat: " << words[i] << ": No such file or directory\n";
            exit_status::set(1);
         }
      }     
   }   
   
}
void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode_ptr curr_wd = state.get_cwd();
   wordvec cwd_path = state.get_path();

   if (words.size() == 1 or (words.size() == 2 and words[1] == "/")) {
      // change the current directory to the root directory
      state.reset_path();
      state.set_cwd(state.get_root());
      return;
   } else if (words.size() == 2) {
      string pathnames = words[1];
      wordvec paths = split(pathnames, "/");
      for (const string &path : paths) {
            if (path == ".") continue;
            inode_ptr to = curr_wd->get_base()->get_mapped_inode_ptr(path);
            if (to == nullptr or 
               (to != nullptr and to->get_base()->get_type() == file_type::PLAIN_TYPE)) {
               cerr << "ls: Invalid path.\n";
               exit_status::set(1);
               return;
            }
            if (path == ".." and not state.is_root(curr_wd)) {
               cwd_path.pop_back();
            } else if (path != "..") {
               cwd_path.push_back(path);
            }
            curr_wd = to;
      }
   }

   state.set_cwd(curr_wd);
   state.set_path(cwd_path);

}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   inode_ptr curr_wd = state.get_cwd();
   wordvec cwd_path = state.get_path();

   if (words.size() == 1) {
      // print the current directory
   } else if (words.size() == 2) {
      string pathnames = words[1];
      if (pathnames == "/") {
         curr_wd = state.get_root();
         cwd_path.clear();
         cwd_path.push_back("/");
      } else {
         wordvec paths = split(pathnames, "/");
         for (const string &path : paths) {
               if (path == ".") continue;
               inode_ptr to = curr_wd->get_base()->get_mapped_inode_ptr(path);
               if (to == nullptr) {
                  cerr << "ls: Invalid path.\n";
                  exit_status::set(1);
                  return;
               }
               if (path == ".." and not state.is_root(curr_wd)) {
                  cwd_path.pop_back();
               } else if (path != "..") {
                  cwd_path.push_back(path);
               }
               curr_wd = to;
         }
      }
   }
   curr_wd->print_path(cwd_path);
   cout << ":" << endl;
   curr_wd->get_base()->print_dirents();
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // make filename stuff
   if (words.size() == 1) {
      cerr << "make: No filename given.\n";
      exit_status::set(1);
      return;
   }
   string filename = words[1];
   inode_ptr curr_wd = state.get_cwd();

   curr_wd = curr_wd->get_base()->mkfile(filename);
   if (curr_wd == nullptr) {
      cerr << "make: Cannot create file with same name as a directory.\n";
      exit_status::set(1);
      return;
   }

   wordvec data_vec;
   for (unsigned int i = 2; i < words.size(); ++i) {
      data_vec.push_back(words[i]);
   }
   curr_wd->get_base()->writefile(data_vec);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() == 1) {
         cerr << "mkdir: No directory name given.\n";
         exit_status::set(1);
         return;
      }

   inode_ptr curr_wd = state.get_cwd();
   string dirname = words[1];
   inode_ptr new_dir = curr_wd->get_base()->mkdir(dirname);
   if (new_dir == nullptr) {
      cerr << "mkdir: Cannot create directory since " << words[1] 
           << " already exists.\n";
      exit_status::set(1);
      return;
   }

   new_dir->get_base()->get_dirents().insert(
      pair<string, inode_ptr>("..", curr_wd));
   new_dir->get_base()->get_dirents().insert(
      pair<string, inode_ptr>(".", new_dir));
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string target = "";
   
   if (words.size() > 1) {
      for (unsigned int i = 1; i < words.size(); ++i) {
         target += words[i] + " ";
      }
   }
   else {
      target = state.inode_state::prompt();
   }

   state.inode_state::set_prompt(target);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   inode_ptr curr_wd = state.get_cwd();
   curr_wd->print_path(state);
   cout << "\n";
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() < 2) {
      cerr << "rm: No filename to delete given.\n";
      exit_status::set(1);
      return;
   }
   string filename = words[1];
   inode_ptr curr = state.get_cwd();
   
   curr->get_base()->remove(filename);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}
