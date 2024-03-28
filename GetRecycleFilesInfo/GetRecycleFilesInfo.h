#include <windows.h>

int WriteOutFile(char * info);
void GetName (STRRET str, LPSTR lpszName);
void HeaderFolder ();
void HeaderFolder2 ();
BOOL GetFolder2 ();
int setQuestionMarkToSpace(char * str);
void FillFolder2 ();
void FillFolder ();
void UpdateList (void);
BOOL GetFolder ();
int GetRecycleFilesInfo (void);
char* findFileInfosInDir(char * dir);
char* findFirstSubdirInDir(char * dir);