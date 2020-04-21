#ifndef _H_HASHLIST_
#define _H_HASHLIST_
 
#define HASH_NUM_MAX 49999
 
#define u32 unsigned int
 
//typedef struct _Node * pNode;
//typedef struct _Hash_Header * pHash_Header;
extern int aaa;

typedef struct _Node
{
	std::string key;
    u32 id;
    std::string value;
    struct _Node *next;        
}Node,*pNode;
 
typedef struct _Hash_Header
{
    struct _Node *next;    
}Hash_Header,*pHash_Header;
 
typedef struct _Hash_List
{
    struct  _Hash_Header* list[HASH_NUM_MAX];           
}Hash_List,*pHash_List;
 
pHash_List init_hash_list(void);
std::string insert_node_to_hash(pHash_List plist,std::string data);
std::string lock_node_to_hash();
std::string unlock_node_to_hash();
void print_hash(pHash_List plist);
std::string get_node_to_hash(pHash_List plist, std::string data);
void free_all_hash(pHash_List plist); 

void init_hash(pHash_List plist); 
 

#endif

