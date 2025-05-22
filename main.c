#include "htwlib.c"
#include "user-htwlib.c"

int main()
{
	user_FMSrun();
	close(memfd);
}
