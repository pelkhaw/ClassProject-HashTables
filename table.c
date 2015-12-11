/*table.c                 
 * Paul El-khawaja           
 * ECE 223 Fall 2015
 * MP7
 *
 * Propose: A template for table.c. 
 */



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "table.h"
#define MINnum 4
#define MAXnum 999999995

/*  The empty table is created.  The table must be dynamically allocated and
 *  have a total size of table_size.  The maximum number of (K, I) entries
 *  that can be stored in the table is table_size-1.  For open addressing, 
 *  the table is filled with a special empty key distinct from all other 
 *  nonempty keys.  
 *
 *  the probe_type must be one of {LINEAR, DOUBLE, QUAD}
 *
 *  Do not "correct" the table_size or probe decrement if there is a chance
 *  that the combinaion of table size or probe decrement will not cover
 *  all entries in the table.  Instead we will experiment to determine under
 *  what conditions an incorrect choice of table size and probe decrement
 *  results in poor performance.
 */
table_t *table_construct(int table_size, int probe_type)
{
        int i;
        table_t * table;
        table =(table_t*) malloc(sizeof(table_t));
        table->table_size_M=table_size;
        table->probeType=probe_type;
        table->numKeys=0;
        table->ProbeCalls=0;
        table->oa=(table_entry_t *)malloc(table_size*sizeof(table_entry_t ));
        for(i=0;i<table_size;i++)
        {                
                table->oa[i].key=0;
                table->oa[i].data_ptr=NULL;
        }

        return table;
}

/* Sequentially remove each table entry (K, I) and insert into a new
 * empty table with size new_table_size.  Free the memory for the old table
 * and return the pointer to the new table.  The probe type should remain
 * the same.
 *
 * Do not rehash the table during an insert or delete function call.  Instead
 * use drivers to verify under what conditions rehashing is required, and
 * call the rehash function in the driver to show how the performance
 * can be improved.
 */
table_t *table_rehash(table_t * T, int new_table_size)
{
        table_t* T2;
        int i;
        hashkey_t k;
        data_t *data;


        T2=table_construct(new_table_size,T->probeType);
        for(i=0;i<T->table_size_M;i++)
        {
                
                if (T->oa[i].key>MINnum && T->oa[i].key<MAXnum)
                {
                        k=T->oa[i].key;
                        data=table_delete(T,k);
                        table_insert(T2,k, data);
                }
        }

        table_destruct(T);        
        return T2;
}

/* returns number of entries in the table */
int table_entries(table_t *T)
{
        return T->numKeys;
}

/* returns 1 if table is full and 0 if not full. */
int table_full(table_t *T)
{
        if ((T->table_size_M)-1==T->numKeys)
                return 1;
        else    
                return 0;
}

/* returns the number of table entries marked as deleted */
int table_deletekeys(table_t *T)
{
        int i;
        int count=0;
        for(i=0;i<T->table_size_M;i++)
        {
                if (T->oa[i].key==1)
                {
                        count++;
                }
        }
        return count;
}
   
/* Insert a new table entry (K, I) into the table provided the table is not
 * already full.  
 * Return:
 *      0 if (K, I) is inserted, 
 *      1 if an older (K, I) is already in the table (update with the new I), or 
 *     -1 if the (K, I) pair cannot be inserted.
 */
int table_insert(table_t *T, hashkey_t K, data_t I)
{
        int M=T->table_size_M;
        int hash=K % M;
        int dec,found,total=0;
        int j=0;
        table_entry_t* entry;
        entry=T->oa;
        if (K>MAXnum||K<MINnum)
                return -1;
        if (T->probeType==LINEAR)
                dec=1;
        else if (T->probeType==DOUBLE)
        {
                dec=(K/M)%M;
                if (dec<1)
                        dec=1;
        }
        T->ProbeCalls=1;
        if (entry[hash].key==K)
        {
                free(entry[hash].data_ptr);
                entry[hash].data_ptr=I;
                return 1;
        }
        if (table_full(T)==1)
                return -1;   
     
        while(entry[hash].key >= MINnum)
        {
                T->ProbeCalls++;
                j++;
                if(T->probeType==QUAD)
                {
                        hash-=j;
                }
                else
                {
                        hash-=dec;
                }
                if(hash<0)
                        hash+=M;
                if (entry[hash].key==K)
                {
                free(entry[hash].data_ptr);
                entry[hash].data_ptr=I;
                return 1;
                }
                total++;
        }
        found=hash;
        if (entry[hash].key==1)
        {
                while(total<M)
                {
                        T->ProbeCalls++;
                        j++;
                        if(T->probeType==QUAD)
                        {
                                hash-=j;
                                //total=total+j;
                        }
                        else
                        {
                                hash-=dec;
                                //total=total+dec;
                        }
                        if(hash<0)
                                hash+=M;
                        if (entry[hash].key==K)
                        {
                        free(entry[hash].data_ptr);
                        entry[hash].data_ptr=I;
                        return 1;
                        }
                        total++;
                        if(total>M)
                                break;
                }
        }
        hash=found;
        T->numKeys++;
        entry[hash].key=K;
        entry[hash].data_ptr=I;

        return 0;
}

/* Delete the table entry (K, I) from the table.  
 * Return:
 *     pointer to I, or
 *     null if (K, I) is not found in the table.  
 *
 * See the note on page 490 in Standish's book about marking table entries for
 * deletions when using open addressing.
 */
data_t table_delete(table_t * T, hashkey_t K)
{
        
        int M=T->table_size_M;
        int hash=K % M;
        int dec,j=0;
        hashkey_t key;
        int count=0;
        table_entry_t* entry;
        entry=T->oa;

        if (T->probeType==LINEAR)
                dec=1;
        else if (T->probeType==DOUBLE)
        {
                dec=(K/M)%M;
                if (dec<1)
                        dec=1;
        }
        key=entry[hash].key;
        T->ProbeCalls=1;
        while((K!=entry[hash].key) &&(entry[hash].key!= 0))
        {
                T->ProbeCalls++;
                j++;
                if(T->probeType==QUAD)
                {
                        hash-=j;
                }
                else
                {
                        hash-=dec;
                }
                if(hash<0)
                        hash+=M;
                key=entry[hash].key;
                if (count>M)
                        return NULL;
                count++;
        }
        
        
        if (key==1||key==0)
                return NULL;
        T->numKeys--;
        entry[hash].key=1;
        return (entry[hash].data_ptr);
}

/* Given a key, K, retrieve the pointer to the information, I, from the table,
 * but do not remove (K, I) from the table.  Return NULL if the key is not
 * found.
 */
data_t table_retrieve(table_t *T, hashkey_t K)
{
        int M=T->table_size_M;
        int hash=K % M;
        int dec;
        hashkey_t key;
        int count=0,j=0;
        table_entry_t* entry;
        entry=T->oa;

        if (T->probeType==LINEAR)
                dec=1;
        else if (T->probeType==DOUBLE)
        {
                dec=(K/M)%M;
                if (dec<1)
                        dec=1;
        }
        key=entry[hash].key;
        T->ProbeCalls=1;

        while((K!=entry[hash].key) &&(entry[hash].key!=0))
        {
                T->ProbeCalls++;
                
                j++;
                if(T->probeType==QUAD)
                {
                        hash-=j;
                }
                else
                {
                        hash-=dec;
                }
                if(hash<0)
                        hash+=M;
                key=entry[hash].key;
                if (key==0)
                        return NULL;
                if(count>M)
                        return NULL;
                count++;
        }
        if (key==1||key==0)
                return NULL;
        return entry[hash].data_ptr;
} 

/* Free all information in the table, the table itself, and any additional
 * headers or other supporting data structures.  
 */
void table_destruct(table_t *T)
{
        
        int i;

        for(i=0;i<(T->table_size_M);i++)
        { 
                if(T->oa[i].key!=1)
                        free(T->oa[i].data_ptr);              
                T->oa[i].data_ptr=NULL;        
                       
        }
        
        free(T->oa);
        free(T);
        
}

/* The number of probes for the most recent call to table_retrieve,
 * table_insert, or table_delete 
 */
int table_stats(table_t *T)
{
        return T->ProbeCalls;
}

/* This function is for testing purposes only.  Given an index position into
 * the hash table return the value of the key if data is stored in this 
 * index position.  If the index position does not contain data, then the
 * return value must be zero.  Make the first
 * lines of this function 
 *       assert(0 <= index && index < table_size); 
 */
hashkey_t table_peek(table_t *T, int index)
{
        assert(0 <= index && index < T->table_size_M); 
        table_entry_t* entry;
        entry=T->oa;
        if (entry[index].key==0 || entry[index].key==1)
                return 0;
        else
                return entry[index].key;
}

/* Print the table position and keys in a easily readable and compact format.
 * Only useful when the table is small.
 */
void table_debug_print(table_t *T)
{
        int i;
        printf("\n");
        for(i=0;i<T->table_size_M;i++)
        {
                if (T->oa[i].key==0)  
                        printf("%i:Empty\n",i);  
                else if (T->oa[i].key==1)  
                        printf("%i:Deleted\n",i);
                else
                        printf("%i:%u\n",i,T->oa[i].key);
        }
}

