// Shell.

#include "types.h"
#include "user.h"
#include "fcntl.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10
#define MAX_HISTORY 16
#define MAX_VAR_SIZE 32
#define MAX_BUF_SIZE 100

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

/// history buf
char history[MAX_BUF_SIZE*MAX_HISTORY];
int historycounter=0;

/// add command to history
void
addToHistory (char* buf)
{
    int index= historycounter%MAX_HISTORY;
    strcpy(history+(index*MAX_BUF_SIZE),buf);
    historycounter++;
}

/// get command from history
char*
getFromHistory (int index)
{
    if (index < 1 || index > 16){
        return 0;
    }
    index--;
    if (historycounter <= MAX_HISTORY){
        return history + (index*MAX_BUF_SIZE);
    }
    else {
        return history + ((historycounter+index)%MAX_HISTORY)*MAX_BUF_SIZE;
    }
}

/// print history
void
printHistory()
{
    int n=historycounter;
    if (n>16)
        n=16;
    int i;
    for(i=1; i<=n ; i++){
        printf(1,"%d. %s",i,getFromHistory(i));
    }  
}


// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    exec(ecmd->argv[0], ecmd->argv);
    printf(2, "exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit();
}

int
getcmd(char *buf, int nbuf)
{
  printf(2, "$ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

char
IsEndOfVar(char c){
  return c == 0 || c == ' ' || c == '\n' || c == '$';
}

char
IsCharValidVar(char c){
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void
HandleCmd(char* buf){
  addToHistory(buf);
  ///will contain the command after replacing $var with value
  char correctedBuf[MAX_BUF_SIZE];
  char varName[MAX_VAR_SIZE];
  char varValue[MAX_BUF_SIZE];
  int i;
  int j;
  int correctedBufIndex = 0;

  /// get var values
  for(i = 0; i < strlen(buf) ; i++){
    if(buf[i] == '$'){
      i++;
      int startVarIndex = i;
      while(!IsEndOfVar(buf[i])){
        if(!IsCharValidVar(buf[i])){
          printf(2, "invalid variable\n");
          return;
        }

        /// check if var name is too long
        if(i - startVarIndex >= MAX_VAR_SIZE){
          printf(2, "variable too long\n");
          return;
        }

        varName[i - startVarIndex] = buf[i];
        i++;
      }

      varName[i - startVarIndex] = 0;
      

      /// get Value
      if(getVariable(varName, varValue) == 0){
        int valueLength = strlen(varValue);

        /// copy value into corrected buffer
        for(j = 0; j < valueLength ; j++){
          correctedBuf[correctedBufIndex++] = varValue[j];
        }
        
        i--;
      }
      else
      {
        printf(2, "variable not found\n");
        return;
      }
    }
    else
    {
      correctedBuf[correctedBufIndex++] = buf[i];
    }
  }

  /// set var value
  for(i = 0 ; i < MAX_BUF_SIZE ; i++){
    if(correctedBuf[i] == '\n'){
      break;
    }

    if(correctedBuf[i] == '='){
      if(i == 0){
        printf(2, "no variable name\n");
        return;
      }

      if(i >= MAX_VAR_SIZE){
        printf(2, "variable too long\n");
        return;
      }

      correctedBuf[strlen(correctedBuf)-1] = 0;
      correctedBuf[i] = 0;
      
      /// var         value
      int setVarRet = setVariable(correctedBuf, correctedBuf + i + 1);
      
      if(setVarRet == -1){
        printf(2, "no room for aditional variables\n");
      }

      if(setVarRet == -2){
        printf(2, "input is illegal\n");
      }

      return;
    }
  }

  if(correctedBuf[0] == 'c' && correctedBuf[1] == 'd' && correctedBuf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      correctedBuf[strlen(correctedBuf)-1] = 0;  // chop \n
      if(chdir(correctedBuf+3) < 0)
        printf(2, "cannot cd %s\n", correctedBuf+3);
      return;
    }
    /// support history
    if(correctedBuf[0] == 'h' && correctedBuf[1] == 'i' && correctedBuf[2] == 's' && correctedBuf[3] == 't' && correctedBuf[4] == 'o' && correctedBuf[5] == 'r' && correctedBuf[6] == 'y'){
        /// history must be called by the parent, not the child.
        if(correctedBuf[7] == '\n'){
          /// print total history
            printHistory();
        }
        else if(correctedBuf[7] == ' ' && correctedBuf[8] == '-' && correctedBuf[9] == 'l' && correctedBuf[10] == ' ')
        {
          /// run history at index
            int index = atoi(correctedBuf+11);
            char cmdText[MAX_BUF_SIZE];
            char* fromHistory = getFromHistory(index);
            if(fromHistory == 0){
                printf(2,"error: invalid hisotry index\n");
                return;
            }
            strcpy(cmdText,fromHistory);
            /// run the command
            HandleCmd(cmdText);
        }
        else
        {
            /// bad args to history
            printf(2, "cannot history %s\n", correctedBuf+7);
        }
      return;
    }
    if(fork1() == 0)
      runcmd(parsecmd(correctedBuf));
    wait();
}

int
main(void)
{  
  static char buf[MAX_BUF_SIZE];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    HandleCmd(buf);
  }
  exit();
}   

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
