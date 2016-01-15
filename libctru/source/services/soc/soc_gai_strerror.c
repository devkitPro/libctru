#include <netdb.h>

const char *gai_strerror(int ecode)
{
	switch(ecode)
	{
		case EAI_FAMILY :
			return "ai_family not supported";
		case EAI_MEMORY :
			return "Memory allocation failure";
		case EAI_NONAME :
			return "Name or service not known";
		case EAI_SOCKTYPE :
			return "ai_socktype not supported";
		default:
			return "Unknown error";
	}
}
