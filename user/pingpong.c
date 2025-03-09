#include "../kernel/types.h"
#include "user.h"

int main(int argc,char* argv[])
{
    int pid;
    int parent_to_child[2], child_to_parent[2];
    char buf[1]={0};
    pipe(parent_to_child);
    pipe(child_to_parent);
    if(fork()==0)
    {
        pid=getpid();
        close(child_to_parent[0]);
        close(parent_to_child[1]);
        read(parent_to_child[0],buf,1);
        close(parent_to_child[0]);
        printf("%d: received ping\n",pid);
        write(child_to_parent[1],"1",1);
        close(child_to_parent[1]);
    }
    else
    {
        pid=getpid();
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        write(parent_to_child[1],"1",1);
        close(parent_to_child[1]);
        wait(0);
        read(child_to_parent[0],buf,1);
        close(child_to_parent[0]);
        printf("%d: received pong\n",pid);
    }
    exit(0);
}