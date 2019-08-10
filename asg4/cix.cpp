// $Id: cix.cpp,v 1.6 2018-07-26 14:18:32-07 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

#include <sstream>
#include <fstream>

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"get" , cix_command::GET },
   {"put" , cix_command::PUT },
   {"rm"  , cix_command::RM  },
};

static const string help = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

void cix_get (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::GET;
   // snprintf formats and stores a series of characters and values
   // in the array buffer.
   // snprintf (char *str, size_t size, const char *format, ...);
   snprintf (header.filename, filename.length() + 1, filename.c_str());
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::FILE) {
      log << filename << " is not on the server" << endl;
      log << "server returned " << header << endl;
   } else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      ofstream fs(filename);
      fs.write(buffer, header.nbytes);
      fs.close();
   }
}

void cix_put (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::PUT;
   if (filename.length() >= FILENAME_SIZE) {
      log << "put: " << filename << ": filename too large" << endl;
      return;
   }
   snprintf (header.filename, filename.length() + 1, filename.c_str());
   ifstream is(filename);
   if (not is) {
      log << "ERROR: could not find file " << filename << endl;    
   } else {
      // get length of file
      is.seekg (0, is.end);
      int length = is.tellg();
      is.seekg (0, is.beg);
      // buffer to read data
      char buffer[length];
      // read data as a block
      is.read (buffer, length);
   
      header.nbytes = length;
      log << "sending header " << header << endl;
      send_packet (server, &header, sizeof header);
      send_packet (server, buffer, length);
      recv_packet (server, &header, sizeof header);
      log << "received header " << header << endl;
   }
   if (header.command == cix_command::NAK) {
      log << "put: NAK received: file failed to be put on server"
          << endl;
   }
   else if (header.command == cix_command::ACK) {
      log << "put: ACK received: file put on server" << endl;   
   }
   is.close();
}

void cix_rm (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::RM;
   header.nbytes = 0;
   snprintf (header.filename, filename.length() + 1, filename.c_str());
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;

   if (header.command == cix_command::NAK) {
      log << "rm: NAK received: failed to delete file" << endl;
   }
   else if (header.command == cix_command::ACK) {
      log << "rm: ACK received: file deletion successful" << endl;
   }

}

void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   vector<string> cmdvec;
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         istringstream ss(line);
         string token;
         while (getline (ss, token, ' ')) {
            cmdvec.push_back(token);
         }
         
         log << "command " << line << endl;
         const auto& itor = command_map.find (cmdvec[0]);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT: {
               throw cix_exit();
               break;
            }
            case cix_command::HELP: {
               cix_help();
               cmdvec.clear();
               break;
            }
            case cix_command::LS: {
               cix_ls (server);
               cmdvec.clear();
               break;
            }
            case cix_command::GET: {
               cix_get (server, cmdvec[1]);
               cmdvec.clear();
               break;
            }
            case cix_command::PUT: {
               cix_put (server, cmdvec[1]);
               cmdvec.clear();
               break;
            }
            case cix_command::RM: {
               cix_rm (server, cmdvec[1]);
               cmdvec.clear();
               break;
            }
            default: {
               log << line << ": invalid command" << endl;
               break;
            }
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

