/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "vfs.h"


u32 vread(vnode_t *node, u32 offset, u32 size, u8 *buffer){
	if(node->read!=NULL){
		return node->read(node,offset,size,buffer);
	}else{
		return 0;
	}
}
u32 vwrite(vnode_t *node, u32 offset, u32 size, u8 *buffer){
	if(node->write!=NULL){
		return node->write(node,offset,size,buffer);
	}else{
		return 0;
	}
}
void vopen(vnode_t *node){
	if(node->open!=NULL){
		return node->open(node);
	}else{
		return ;
	}
}
void vclose(vnode_t *node){
	if(node->close!=NULL){
		return node->close(node);
	}else{
		return ;
	}
}
struct vdirent *vreaddir(vnode_t *node, u32 index){
	if((node->flags&0x7) == V_DIRECTORY&&node->readdir!=NULL){
		return node->readdir(node,index);
	}else{
		return 0;
	}
}
vnode_t *vfinddir(vnode_t *node, char *name){
	if((node->flags&0x7) == V_DIRECTORY&&node->finddir!=NULL){
		return node->finddir(node,name);
	}else{
		return 0;
	}
}
