#include <2440/ports.h>
#include <ecasey/kernel.h>
#include <string.h>
#include <sys/types.h>

/* 
 * GENERNAL node funcs
 * this is copied from linux some comment added by casey 
 * 2013 - 5 - 10 
 */
struct list_head {
	struct list_head *next, *prev;
};

/*
 *      list-->list
 *      list<--list
 */      

LOCAL INLINE void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 *      prev-->next
 *      prev<--next
 *
 *      prev-->newlist-->next
 *      prev<--newlist<--next
 */
LOCAL INLINE void __list_add(struct list_head *new_lst,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new_lst;
	new_lst->next = next;
	new_lst->prev = prev;
	prev->next = new_lst;
}

/*
 *      add node after head 
 */
LOCAL INLINE void list_add(struct list_head *new_lst, struct list_head *head)
{
	__list_add(new_lst, head, head->next);
}

/*      
 *      add node after tail
 */
LOCAL INLINE void list_add_tail(struct list_head *new_lst, struct list_head *head)
{
	__list_add(new_lst, head->prev, head);
}

/*
 *      prev-->del-->next
 *      prev<--del<--next
 *
 *      prev-->next
 *      prev<--next
 */
LOCAL INLINE void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/*
 *      del entry 
 */
LOCAL INLINE void list_del(struct list_head * entry)
{
	__list_del(entry->prev,entry->next);
}


/*      del chain from (ch-...-ct) to list      
 *      x-->ch-->ct-->y
 *      x<--ch<--ct<--y
 *
 *      x-->y
 *      x<--y
 */
LOCAL INLINE void list_remove_chain(struct list_head *ch,struct list_head *ct)
{

	ch->prev->next=ct->next;
	ct->next->prev=ch->prev;
}
/*
 *      head-->x
 *      head<--x
 *
 *      head-->ch-->ct-->x
 *      head<--ch<--ct<--x
 */
LOCAL INLINE void list_add_chain(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
	ch->prev=head;
	ct->next=head->next;
	head->next->prev=ct;
	head->next=ch;
}
/*      head-->x
 *      head<--x
 *
 *      head-->x-->ch-->ct
 *      head<--x<--ch<--ct
 */
LOCAL INLINE void list_add_chain_tail(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
	ch->prev=head->prev;
	head->prev->next=ch;
	head->prev=ct;
	ct->next=head;
}
/*
 *      head-->head
 *      head<--head
 */
LOCAL INLINE i32 list_empty(RO struct list_head *head)
{
	return head->next == head;
}

#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)

/*
 * to get struct address from ptr in struct type's member 
 * THIS IS SO GOOD !! ^_^
 */
#define container_of(ptr, type, member) ({			\
	RO typeof( ((type *)0)->member ) *__mptr = (ptr);	\
		(type *)( (i8 *)__mptr - offsetof(type,member) );})

#define list_entry(ptr,type,member)	\
    container_of(ptr, type, member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* FIXME*/
#define _MEM_END	        MAP_AREA_END 
#define _MEM_START	        MAP_AREA_START

#define PAGE_SHIFT	        (12)
#define PAGE_SIZE	        (1<<PAGE_SHIFT)
#define PAGE_MASK	        (~(PAGE_SIZE-1))

#define KERNEL_MEM_END	    (_MEM_END)
/*
 * the begin and end of the kernel mem which is needed to be paged.
 */
#define KERNEL_PAGING_START	((_MEM_START+(~PAGE_MASK))&((PAGE_MASK)))               /* page align */
#define	KERNEL_PAGING_END	(KERNEL_MEM_END)

/*
 * page number in need 
 */
#define KERNEL_PAGE_NUM	    (PAGE_STRUCTS_NR)
/*
 * the start and end of the page structure should be storaged in.*
 */
#define KERNEL_PAGE_END	    (PAGE_STRUCTS_START+PAGE_STRUCTS_SIZE)
#define KERNEL_PAGE_START	(PAGE_STRUCTS_START)

/*page flags*/
#define PAGE_AVAILABLE		0x00
#define PAGE_DIRTY			0x01
#define PAGE_PROTECT		0x02
#define PAGE_BUDDY_BUSY		0x04
#define PAGE_IN_CACHE		0x08

/* total 7 * 4 bytes FIXME when this is changed !!! */
struct page {
	u32     vaddr;
	u32     flags;
	i32     order;
	u32     counter;
	struct  kmem_cache *cachep;
	struct  list_head list;      //to string the buddy member
};

#define MAX_BUDDY_PAGE_NUM	        (9)	// 1M alloced 
#define AVERAGE_PAGE_NUM_PER_BUDDY	(KERNEL_PAGE_NUM/MAX_BUDDY_PAGE_NUM)
#define PAGE_NUM_FOR_MAX_BUDDY	    ((1<<MAX_BUDDY_PAGE_NUM)-1)

struct list_head *page_buddy = (void*)0;
/*
 * this is used just for kernel space 
 */
struct page *virt_to_page(u32 addr)
{
	u32 i;
	i=((addr)-KERNEL_PAGING_START)>>PAGE_SHIFT;
	if(i>KERNEL_PAGE_NUM)
		return NULL;
	return (struct page *)KERNEL_PAGE_START+i;
}

void init_page_map(void)
{
    page_buddy = (struct list_head*)(KERNEL_PAGE_END);

	i32 i;
	struct page *pg=(struct page *)KERNEL_PAGE_START;
	// init buddy array first       
	for(i=0;i<MAX_BUDDY_PAGE_NUM;i++)
		INIT_LIST_HEAD(&page_buddy[i]);
	for(i=0;i<(KERNEL_PAGE_NUM);pg++,i++){
        // fill struct page then 
		pg->vaddr=KERNEL_PAGING_START+i*PAGE_SIZE;	
		pg->flags=PAGE_AVAILABLE;
		pg->counter=0;
		INIT_LIST_HEAD(&(pg->list));
        // make the memory max buddy as possible
		if(i<(KERNEL_PAGE_NUM&(~PAGE_NUM_FOR_MAX_BUDDY))){	
            // the following code should be dealt carefully
            // we would change the order field of a head 
            // struct page to the corresponding order 
            // and change others to -1
			if((i&PAGE_NUM_FOR_MAX_BUDDY)==0)
				pg->order=MAX_BUDDY_PAGE_NUM-1;
			else
				pg->order=-1;
			list_add_tail(&(pg->list),&page_buddy[MAX_BUDDY_PAGE_NUM-1]);
        //the remainder not enough to merge i32o a max buddy is done as min buddy
		} else {
			pg->order=0;
			list_add_tail(&(pg->list),&page_buddy[0]);
		}
	}
}
/*
 * we can do these all because the page structure 
 * that represents one page aera is continuous
 */
#define BUDDY_END(x,order)	        ((x)+(1<<(order))-1)
#define NEXT_BUDDY_START(x,order)	((x)+(1<<(order)))
#define PREV_BUDDY_START(x,order)	((x)-(1<<(order)))
/*
 * the logic of this function seems good,no bug reported yet
 */
struct page *get_pages_from_list(i32 order)
{
	i32 neworder=order;
	struct page *pg;
	struct list_head *tlst,*tlst1;
	for(;neworder<MAX_BUDDY_PAGE_NUM;neworder++){
		if(list_empty(&page_buddy[neworder])){      // no free pages in this order 
			continue;                               // next order 
		}else{
			pg=list_entry(page_buddy[neworder].next,struct page,list);  // get it 
			tlst=&(BUDDY_END(pg,neworder)->list);   // mark it 
			tlst->next->prev=&page_buddy[neworder]; // del tlst
			page_buddy[neworder].next=tlst->next;   // ptr to next 
			goto OUT_OK;                            // get out 
		}
	}
	return NULL;
OUT_OK:                                             // cut the order 
	for(neworder--;neworder>=order;neworder--){     // till fill up neworder
		tlst1=&(BUDDY_END(pg,neworder)->list);         
		tlst=&(pg->list);                           
		pg=NEXT_BUDDY_START(pg,neworder);
		list_entry(tlst,struct page,list)->order=neworder;
		list_add_chain_tail(tlst,tlst1,&page_buddy[neworder]);
	}
	pg->flags|=PAGE_BUDDY_BUSY;                     // page busy now
	pg->order=order;                                // set nre order after cutting off
	return pg;
}

void put_pages_to_list(struct page *pg,i32 order)
{
	struct page *tprev,*tnext;
	if(!(pg->flags&PAGE_BUDDY_BUSY))                // non-alloced page
		return;
	pg->flags&=~(PAGE_BUDDY_BUSY);

	for(;order<MAX_BUDDY_PAGE_NUM;order++){
		tnext=NEXT_BUDDY_START(pg,order);
		tprev=PREV_BUDDY_START(pg,order);
		if((!(tnext->flags&PAGE_BUDDY_BUSY))&&(tnext->order==order)){
			pg->order++;                            // can be linked
			tnext->order=-1;
			list_remove_chain(&(tnext->list),&(BUDDY_END(tnext,order)->list));
			BUDDY_END(pg,order)->list.next=&(tnext->list);
			tnext->list.prev=&(BUDDY_END(pg,order)->list);
			continue;
		}else if((!(tprev->flags&PAGE_BUDDY_BUSY))&&(tprev->order==order)){
			pg->order=-1;                           // can be linked 
			
			list_remove_chain(&(pg->list),&(BUDDY_END(pg,order)->list));
			BUDDY_END(tprev,order)->list.next=&(pg->list);
			pg->list.prev=&(BUDDY_END(tprev,order)->list);
			
			pg=tprev;
			pg->order++;
			continue;
		}else{                                      // can not be linked
			break;
		}
	}
	list_add_chain(&(pg->list),&((tnext-1)->list),&page_buddy[order]);  // add to chain
}

void *page_address(struct page *pg)
{
	return (void *)(pg->vaddr);
}

struct page *alloc_pages(u32 flag,i32 order)
{
	struct page *pg;
	i32 i;
	pg=get_pages_from_list(order);
	if(pg==NULL)
		return NULL;
	for(i=0;i<(1<<order);i++){
		(pg+i)->flags|=PAGE_DIRTY;
	}
	return pg;
}

void free_pages(struct page *pg,i32 order)
{
	i32 i;
	for(i=0;i<(1<<order);i++)
		(pg+i)->flags&=~PAGE_DIRTY;
	put_pages_to_list(pg,order);
}

void *get_free_pages(u32 flag,i32 order)
{
	struct page * page;
	page = alloc_pages(flag, order);
	if (!page)
		return NULL;
    void *addr = page_address (page);
    memset (addr,0,1<<(12+order));
	return	(addr);
}

void put_free_pages(void *addr,i32 order)
{
	free_pages(virt_to_page((u32)addr),order);
}

u32 find_free_page (void)
{
    return ((u32)get_free_pages (0,0));
}

u32 find_free_page_dir ( void )
{
    i32 addr,order = 2;

    while ((0x3FFF & (addr = (u32)get_free_pages (0,order))))
        put_free_pages ((void*)addr,order);
    return (addr);
}

/* get buddy order from blk size */ 
i32 get_order (u32 size)
{
    if (!size) return (-1);
    i32 order = 0;
    while ( (1<<(order+12)) < size ) order ++;
    if (order > 9)  
        return (-1);
    return (order);
}
