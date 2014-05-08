/* Algorithms are fun!
 * Name: myass2.c
 * Author: Mubashwer Salman Khurshid (mskh, 601738)
 * Date: 5/10/2013
 * Description: Preferential Voting (Project 2)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* macros */
#define NAMELEN       30            /* maximum length of any input line */
#define CH_NULL       '\0'          /* character null */
#define CH_NEWLINE    '\n'          /* character new line */
#define FALSE         (-1)          /* false value of some variables/functions*/
#define TRUE          (1)           /* true value of some variables/functions*/
#define FIFTY_PERCENT (50)          /* percent of votes needed to win a round */


/******************************************************************************/
/****************************DATA STRUCTURES***********************************/
/******************************************************************************/

/* Each node represents a vote */
typedef struct node
{
    int prefOrd; /* preference of candidate */
    int candID;
    struct node *next;
} node_t;


/* Each link list represents a voter in which the votes are arranged in the
   order of preference. */
typedef struct link_list
{
    node_t *head;
} list_t;


/* Each struct represents a candidate */
typedef struct
{
    char name[NAMELEN];
    list_t **voterListArr; /* Dynamic array of lists; each list contains all
                              votes by particular voter. Votes are arranged in
                              order of preference. */
    int votes; /* Number of votes won */
    int eliminated;

} cand_t;

/******************************************************************************/
/***************************FUNCTION PROTOTYPES********************************/
/******************************************************************************/

list_t * make_list();
void insert_vote(list_t *myList, int candID, int prefOrd);
void delete_vote(list_t *myList, int candID);
void delete_list(list_t *myList);
void insert_voter(cand_t *candidates, int victorID, list_t *voter);
void deallocate_memory(cand_t *candidates, int totalCands);
int find_losers(cand_t *candidates, int totalCands, int totalVotes,
                int *eliminateIDs);
int print_result(cand_t **cand_pointers, int totalCands, int totalVotes,
                 int round, int width);
int is_less(const void* cand1, const void* cand2);
void transfer_votes(cand_t *candidates, int totalCands, int eliminatID);
void preferencing(cand_t *candidates, int totalCands,int totalVotes, int width);

/******************************************************************************/
/********************************FUNCTIONS*************************************/
/******************************************************************************/

/* Allocates memory for list
 */
list_t * make_list()
{
    list_t *myList = malloc(sizeof(myList));
    assert(myList != NULL);
    myList->head = NULL;
    return myList;
}

/******************************************************************************/

/* This functions inserts vote into the list in the order of its preference
 */
void insert_vote(list_t *myList, int candID, int prefOrd)
{
    node_t *current = myList->head;
    node_t *previous = myList->head;

    node_t *newNode = malloc(sizeof(node_t));
    assert(newNode != NULL);
    newNode->prefOrd = prefOrd;
    newNode->candID = candID;
    newNode->next = NULL;

    /* if list is empty, head points to new node*/
    if (myList->head == NULL)
    {
        myList->head = newNode;
        return ;
    }

    /* the list is iterated until higher preference is found.
       Running time in this process is 0(m) */
    while ((current != NULL) && (prefOrd >= current->prefOrd))
    {
        previous = current;
        current = current->next;
    }

    /* if end of list is reached, the new node is connected to the last node
       otherwise the node is placed in the appropriate position */
    if (current == NULL)
        previous->next = newNode;
    else
    {
        newNode->next = current;
        if (current == myList->head) /*if there is prev. only one node in list*/
            myList->head = newNode;
        else
            previous->next = newNode;
    }
    return;
}


/******************************************************************************/

/*It deletes the node with given candID from list if it is present
 */
void delete_vote(list_t *myList, int candID)
{
    node_t *current = myList->head;
    node_t *previous = myList->head;

    /* the list is iterated untill the node to be deleted is found. Max. running
       time in this process is 0(m) */
    while(current != NULL && candID != current->candID)
    {
        previous = current;
        current = current->next;
    }
    if (current != NULL)
    {
        /* if the  note to be deleted is the first node..*/
        if(current == myList->head)
            myList->head = myList->head->next;
        else
            previous->next = current->next;
        free(current);
    }
    return;
}

/******************************************************************************/

/* It deletes the entire list
 */
void delete_list(list_t *myList)
{
    node_t *previous = NULL;
    node_t *current = myList->head;

    /*Each node is deleted as the list is iterated. Running time in this
      process is 0(m) */
    while(current != NULL)
    {
        previous = current;
        current = current->next;
        free(previous);
    }
    return;
}

/******************************************************************************/

/* It assigns a list of votes of a voter to the voter's first preference
 * candidate
 */
void insert_voter(cand_t *candidates, int victorID, list_t *voter)
{
    int votes;
    candidates[victorID].votes++; /* number of votes won is increased */
    votes = candidates[victorID].votes;

    /* Additional memory is allocated and the new list of votes from a new
       voter is added to the voterListArr */
    candidates[victorID].voterListArr
        = realloc(candidates[victorID].voterListArr, sizeof(list_t *) * votes);
    assert(candidates[victorID].voterListArr != NULL);

    candidates[victorID].voterListArr[votes-1] = voter;
    return;
}

/******************************************************************************/

/* Memory allocated for the candidates and voters are freed
 */
void deallocate_memory(cand_t *candidates, int totalCands)
{
    int i, j;
    /* Each list of votes are deleted. Running time in this process is 0(mn) */
    for(i = 0; i < totalCands; i++)
        for(j = 0; j < candidates[i].votes; j++)
            delete_list(candidates[i].voterListArr[j]);

    free(candidates);
    return;
}

/******************************************************************************/

/* It finds the candidate(s) with minimum votes and returns the number of such
 * candidates.
 */
int find_losers(cand_t *candidates, int totalCands, int totalVotes,
                int *eliminateIDs)
{
    int i, minVote = -1, j = 0, equalLastCount = 1;

    /* Running time in this process is 0(m) */
    for(i = 0; i < totalCands; i++)
    {
        if(candidates[i].eliminated == TRUE)
            continue;

        /* In the first iteration or everytime a lower vote count is found
           eliminatID is updated after its index is reset. minVote is initially
            -1 so that this block is entered in first iteration. */
        if (candidates[i].votes <  minVote || minVote == -1)
        {
            j = 0;
            equalLastCount = 1;
            minVote = candidates[i].votes;
            eliminateIDs[j] = i;

        }
        /* When mulitple candidates with possible minimum vote is found,
           candidateId is added to eliminateIDs */
        else if(candidates[i].votes == minVote)
        {
            eliminateIDs[++j] = i;
            equalLastCount++;
        }
    }
    return equalLastCount;
}

/******************************************************************************/

/* It generates output
 */
int print_result(cand_t **cand_pointers, int totalCands, int totalVotes,
                 int round, int width)
{
    int i, elected = FALSE;
    double votePc;
    if(round == 1)
        printf("%d candidates, %d votes\n", totalCands, totalVotes);

    printf("\nRound %d ...\n", round);
    /* Running time in this process is 0(m) */
    for(i = 0; i < totalCands; i++)
    {
        if(cand_pointers[i]->eliminated == TRUE)
            continue;
        /* Percentage of votes won by candidate is calculated */
        votePc = (cand_pointers[i]->votes / (double) totalVotes) * 100.0;

        printf("%-*s:", width, cand_pointers[i]->name);
        printf("%5d votes, %4.1f%%", cand_pointers[i]->votes, votePc);

        if(votePc > FIFTY_PERCENT) /* If cand. has > 50% votes, he wins */
        {
            elected = TRUE;
            printf("  *** elected");
        }
        printf("\n");
    }
    return elected;
}

/******************************************************************************/

/* It is the comparison function needed for system qsort which compares
 * two candidate pointers. Candidates are to be sorted in descending order of
 * votes obtained and in alphabetical order if multiple candidates have same
 * number of votes
 */
int is_less(const void* cand1, const void* cand2)
{
    if ((*(cand_t **)cand1)->votes >  (*(cand_t **)cand2)->votes)
        return FALSE;
    if ((*(cand_t **)cand1)->votes <  (*(cand_t **)cand2)->votes)
        return TRUE;
    return strcmp((*(cand_t **)cand1)->name, (*(cand_t **)cand2)->name);
}

/******************************************************************************/

/* It transfers the list(s) of votes to the next highest preference candidate
 * when a candidate is eliminated
 */
void transfer_votes(cand_t *candidates, int totalCands, int eliminatID)
{
    int i, j, transferID;
    list_t *voter;

    /* Votes given to the eliminated cand. are removed from and they are
       assigned to the next highest preference candidate. Maximum running time of
       this process is O(n) */
    for(i = 0; i < candidates[eliminatID].votes; i++)
    {
        voter = candidates[eliminatID].voterListArr[i];
        delete_vote(voter, eliminatID);
        transferID = voter->head->candID;
        insert_voter(candidates, transferID, voter);
    }
    candidates[eliminatID].eliminated = TRUE;
    candidates[eliminatID].votes = 0;

    /* Votes given to eliminated cand. are removed from all lists of votes */
    /* Running time in this process is 0(mn) */
    for(i = 0; i < totalCands; i++)
        for(j = 0; j < candidates[i].votes; j++)
            delete_vote(candidates[i].voterListArr[j], eliminatID);
    return;
}

/******************************************************************************/

/* The elimination process
 */
void preferencing(cand_t *candidates, int totalCands, int totalVotes, int width)
{
    cand_t **cand_pointers = malloc(sizeof(cand_t *) * totalCands);
    int round = 1, equalLastCount = 1, elected = FALSE, i;
    int *eliminateIDs = malloc(sizeof(int) * totalCands);
    assert(cand_pointers != NULL && eliminateIDs != NULL);
    srand(time(NULL));

    /*cand_pointers are used to sort candidates and print output as sorting
    actual candidate data would change their ID which are indices of array */
    for(i = 0; i < totalCands; i++)
        cand_pointers[i] = &candidates[i];

    while(elected == FALSE)
    {
        /* When there is 2 or more equal last candidates, one such candidate
           is randomly eliminated */
        if(equalLastCount > 1)
            eliminateIDs[0] = eliminateIDs[(rand() % equalLastCount)];

        /* Votes are transferred after candidate to be eliminated is found */
        if(round > 1)
            transfer_votes(candidates, totalCands, eliminateIDs[0]);

        qsort(cand_pointers, totalCands, sizeof(cand_t *), is_less);

        /* Candidate(s) who could be eliminated are identified */
        equalLastCount = find_losers(candidates, totalCands, totalVotes,
                                     eliminateIDs);
        /* Prints output */
        elected = print_result(cand_pointers, totalCands, totalVotes, round,
                               width);
        if(equalLastCount > 1)
            printf("%d equal last candidates, outcome determined randomly\n",
                    equalLastCount);
        round++;
    }

    /* deallocation of memory */
    free(eliminateIDs);
    free(cand_pointers);
    return;
}

/******************************************************************************/

/* main program reads data from stdin and controls all the action
 */
int main(int argc, char **argv)
{
    char c;
    int candID = 0, j, totalCands, totalVotes = 0, prefOrd, victorID, width = 0;
    cand_t *candidates;
    list_t *voter = make_list();

    scanf("%d\n", &totalCands);
    candidates = malloc(sizeof(cand_t) * totalCands);
    assert(candidates != NULL);

    /* Candidate names are stored and other variables of each candidate
       struct are initialized */
    for(j = 0 ; candID < totalCands; j = 0, candID++)
    {
        while ((c=getchar())!= CH_NEWLINE)
            candidates[candID].name[j++] = c;

        candidates[candID].name[j] = CH_NULL;
        if(j > width)
            width = j;  /*width for printing names in output*/

        candidates[candID].votes = 0;
        candidates[candID].voterListArr = NULL;
        candidates[candID].eliminated = FALSE;
    }

    candID = 0;
    /* Preference values for each candidate by every voter is read */
    while(scanf("%d", &prefOrd) == 1)
    {
        /* Each vote is added to a list of votes for particular voter */
        insert_vote(voter, candID, prefOrd);
        /* VictorID is the ID of cand. who is the first pref. of the voter*/
        if(prefOrd == 1)
            victorID = candID;

        candID++;
        /* When all the votes of the particular voter is stored, the list of
           votes are assigned to the first-pref candidate */
        if(candID == totalCands)
        {
            totalVotes++;
            candID = 0;
            insert_voter(candidates, victorID, voter);
            voter = make_list();
        }
    }
    /* Elimination process */
    preferencing(candidates, totalCands, totalVotes, width);

    deallocate_memory(candidates, totalCands);
    return 0;
}
