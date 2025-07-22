

#ifndef	LINKED_LIST_H
#define LINKED_LIST_H

/************************************************************************************/
typedef struct LINKED_NODE
{
	struct LINKED_NODE * p_next;
	struct LINKED_NODE * p_previous;
	void *               p_data;
} LINKED_NODE;

/************************************************************************************/
typedef struct LINKED_LIST
{
	LINKED_NODE *       p_first_node;
	LINKED_NODE *		p_last_node;
	void *			    list_semMutex;
} LINKED_LIST;


#ifdef __cplusplus
extern "C" {
#endif

HT_API LINKED_LIST* h_list_create(BOOL bNeedMutex);
HT_API void         h_list_free_container(LINKED_LIST * p_linked_list);
HT_API void         h_list_free_all_node(LINKED_LIST * p_linked_list);

HT_API void         h_list_get_ownership(LINKED_LIST * p_linked_list);
HT_API void         h_list_giveup_ownership(LINKED_LIST * p_linked_list);

HT_API BOOL         h_list_remove(LINKED_LIST * p_linked_list, LINKED_NODE * p_node);
HT_API BOOL         h_list_remove_data(LINKED_LIST * p_linked_list, void * p_data);

HT_API void         h_list_remove_from_front(LINKED_LIST *  p_linked_list);
HT_API void         h_list_remove_from_front_no_lock(LINKED_LIST * p_linked_list);
HT_API void         h_list_remove_from_back(LINKED_LIST * p_linked_list);

HT_API BOOL         h_list_add_at_front(LINKED_LIST * p_linked_list, void * p_item);
HT_API BOOL         h_list_add_at_back(LINKED_LIST * p_linked_list, void * p_item);

HT_API uint32       h_list_get_number_of_nodes(LINKED_LIST * p_linked_list);

HT_API BOOL         h_list_insert(LINKED_LIST * p_linked_list, LINKED_NODE * p_pre_node, void * p_item);

HT_API LINKED_NODE* h_list_lookup_start(LINKED_LIST * p_linked_list);
HT_API LINKED_NODE* h_list_lookup_next(LINKED_LIST * p_linked_list, LINKED_NODE * p_node);
HT_API void         h_list_lookup_end(LINKED_LIST * p_linked_list);

HT_API LINKED_NODE* h_list_lookback_start(LINKED_LIST * p_linked_list);
HT_API LINKED_NODE* h_list_lookback_next(LINKED_LIST * p_linked_list, LINKED_NODE * p_node);
HT_API void         h_list_lookback_end(LINKED_LIST * p_linked_list);

HT_API BOOL         h_list_is_empty(LINKED_LIST * p_list);
HT_API LINKED_NODE* h_list_get_from_front(LINKED_LIST * p_list);
HT_API LINKED_NODE* h_list_get_from_back(LINKED_LIST * p_list);

#ifdef __cplusplus
}
#endif

#endif // LINKED_LIST_H


