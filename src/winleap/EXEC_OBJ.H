// exec_obj.h
#include <windows.h>
#include <vcl\vcl.h>

//#define debug
#ifdef debug
  #include <logger.h>
  logger olog("C:\\projects\\exec_obj\\exec.log");
  #endif

class exec_obj {
  private:
    HANDLE process_id,  // handle of created process
           inhand,      // file handles
           outhand,
           errhand;
    STARTUPINFO stup;
    char * cmdline;
    char * startdir;
    bool create_console, minimize;
    DWORD timeout;

    void init();

  public:
    // con and de structors
    exec_obj();         // default constructor
    exec_obj(char command_line[]);
    ~exec_obj();

    // public methods that execute the process
    bool execute_registered(const char filename[]);
    bool execute_registered(const AnsiString & filename);
    bool execute(const char cmdline[]);
    bool execute(const AnsiString & cmdline);

    // public methods that setup something
    void set_startdir(const char dir[]);
    void set_title(char * title);
    void set_create_console(bool create);
    void set_minimize(bool min);
    void set_timeout(long time);
    HANDLE get_process_id();
    HANDLE get_stdin_pipe(); // returns handle to writer of stdin
    HANDLE get_stdout_pipe();
    HANDLE get_stderr_pipe();
    bool process_active();

  };
