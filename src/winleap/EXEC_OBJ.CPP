#include "exec_obj.h"
#include <assert.h>

void exec_obj::init() {
  startdir = NULL;
  process_id = NULL;
  cmdline = NULL;
  create_console=false;
  minimize=true;
  outhand = NULL;
  inhand = NULL;
  errhand = NULL;
  stup.cb = sizeof stup;
  stup.lpTitle = NULL;
  stup.lpDesktop = NULL; // only needed for Win NT, but ignored by 95
  stup.dwFlags = STARTF_USESHOWWINDOW;
  stup.hStdInput = NULL;
  stup.hStdOutput = NULL;
  stup.hStdError = NULL;
  stup.lpReserved2 = NULL;
  stup.cbReserved2 = 0;
  stup.dwX = stup.dwY = 0;
  stup.dwXSize = 200;
  stup.dwYSize = 160;
  stup.wShowWindow = SW_SHOWDEFAULT;
  timeout = INFINITE;
  }

exec_obj::exec_obj() {
  init();
  }

exec_obj::exec_obj(char command_line[]){
  init();
  int len=strlen(command_line);
  cmdline = new char[len+2];
  strcpy(cmdline,command_line);
  }

exec_obj::~exec_obj() {
  if(startdir != NULL)
    delete [] startdir;
  if(cmdline != NULL)
    delete [] cmdline;
  if(process_id != NULL)
    CloseHandle(process_id);
  if(inhand != NULL)
    CloseHandle(inhand);
  if(outhand != NULL)
    CloseHandle(outhand);
  if(errhand != NULL)
    CloseHandle(errhand);
  }

void exec_obj::set_startdir(const char dir[]) {
   int len = strlen(dir);
   if(startdir != NULL)
     delete [] startdir;
   startdir = new char[len+2];
   strcpy(startdir,dir);
   }

void exec_obj::set_title(char * title){
  stup.lpTitle = title;
  }

void exec_obj::set_create_console(bool create) {
  create_console = create;
  }

void exec_obj::set_minimize(bool min){
  minimize = min;
  }

HANDLE exec_obj::get_process_id(){
  return process_id;
  }


void exec_obj::set_timeout(long time){
  timeout = time;
  }

bool exec_obj::process_active(){
  DWORD ec;
  if(GetExitCodeProcess(process_id,&ec)){
     return ec==STILL_ACTIVE;
     }
  return false;
  }

HANDLE exec_obj::get_stdin_pipe() {
  HANDLE r,w;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof sa;
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = true;
  int res = CreatePipe(&r,&w,&sa,0);
  assert(res!=0);
  inhand = r;
//  if(SetStdHandle(STD_INPUT_HANDLE,r))
//    return w;
  return w;
//  return NULL;
  }

HANDLE exec_obj::get_stdout_pipe(){
  HANDLE r,w;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof sa;
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = true;
  int res = CreatePipe(&r,&w,&sa,12000);
  assert(res!=0);
  outhand = w;
//  if(SetStdHandle(STD_OUTPUT_HANDLE,w))
  return r;
//  return NULL;
  }

HANDLE exec_obj::get_stderr_pipe(){
  HANDLE r,w;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof sa;
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = true;
  int res = CreatePipe(&r,&w,&sa,0);
  assert(res!=0);
  errhand = w;
  if(SetStdHandle(STD_ERROR_HANDLE,w))
    return r;
  return NULL;
  }

bool exec_obj::execute(const char command_line[]) {
 if(command_line!=NULL && cmdline == NULL) {
   delete [] cmdline;
   cmdline = new char[strlen(command_line)];
   strcpy(cmdline,command_line);
   }
 if(inhand || outhand || errhand)
     stup.dwFlags |= STARTF_USESTDHANDLES;
 stup.hStdInput = inhand;
 stup.hStdOutput = outhand;
 stup.hStdError = errhand;
 if (minimize)
    stup.wShowWindow = SW_MINIMIZE;
 PROCESS_INFORMATION p_info;
 DWORD create_flags = 0;
 if(create_console) {
   create_flags |= CREATE_NEW_CONSOLE;
   }
 if(timeout==INFINITE)
   create_flags |= HIGH_PRIORITY_CLASS; // if we must wait, lets go faster
 else
   create_flags |= NORMAL_PRIORITY_CLASS;
 bool ok = CreateProcess(NULL,cmdline,NULL,NULL,true,create_flags,
              NULL, startdir, &stup, &p_info);
 if(ok){
    WaitForSingleObject(p_info.hProcess,timeout);
    process_id = p_info.hProcess;
    }
 return ok;
}

bool exec_obj::execute(const AnsiString & command_line) {
 return execute(command_line.c_str());
}

bool exec_obj::execute_registered(const char filename[]) {

 HINSTANCE h = ShellExecute(NULL,     // handle to parent window
                            "open",   // pointer to string that specifies operation to perform
                            filename, // pointer to filename or folder name string
                            NULL,	  // pointer to string that specifies executable-file parameters
                            NULL,     // pointer to string that specifies default directory
                            SW_SHOW   // whether file is shown when opened
                            );

 return int(h) > 32;
}


bool exec_obj::execute_registered(const AnsiString & command_line) {
 return execute_registered(command_line.c_str());
}
