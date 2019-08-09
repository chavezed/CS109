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
   {"#"     , fn_cmt   },
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
      cout << ": No such file or directory" << endl;
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
         }
      }     
   }   
   
}
void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
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

   inode_ptr curr_wd = state.get_cwd();;
   
   // only ls command was entered
   if (words.size() == 1 || words[1] == ".") {
      curr_wd->print_path();  
   }
   else {
      if (words[1] == "/") {
         // root directory
         curr_wd = state.get_root();
         curr_wd->print_path();
      }
      else if (words[1] == "..") {
         // parent directory
         curr_wd = curr_wd->get_parent();
         curr_wd->print_path();
      }
   }
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
   string filename = words[1];
   inode_ptr curr_wd = state.get_cwd();
   string data_str = "";
   wordvec data_vec;
   for (unsigned int i = 2; i < words.size(); ++i) {
      if (i == words.size() - 1) {
         data_str += words[i];
      }
      else {
         data_str += words[i] + " ";
      }
   }
   data_vec.push_back(data_str);
   curr_wd = curr_wd->get_base()->mkfile(filename);
   curr_wd->get_base()->writefile(data_vec);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode_ptr curr_wd = state.get_cwd();
   
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string target;
   
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
   curr_wd->print_path();

}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_cmt (inode_state& state, const wordvec& words){
   
}
