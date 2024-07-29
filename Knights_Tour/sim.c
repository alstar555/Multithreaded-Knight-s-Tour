#define _GNU_SOURCE
#define MAX_THREADS 100000

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h> 
#include <string.h>

extern long next_thread_number;
extern int max_squares;
extern int total_open_tours;
extern int total_closed_tours;

int m, n; //board size
int org_r, org_c; //start positions
long* ThreadIDLookUp; //array of all thread id
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //init mutex


void *findTour(void *_args); //prototype
void print_board(int r, int c, int visited_count, int** visited_places, int total_squares, int move);


typedef struct thread_data{
    long thread_id;
    int squares_visited;
    int** visited_places;
    int r;
    int c;
    int** valid_moves;
    int visited_count;
    int total_squares;
    int move;

}thread_data;

//look up thread id and remove from list
long thread_id_lookup(long id) {
	for(int i = 0; i < MAX_THREADS; i++) {
		if(*(ThreadIDLookUp+i) == id) {
            *(ThreadIDLookUp+i) = -1;
			return i;
		}
	}
	return -1;
}


void step(int* move, int * valid_moves, int* r, int* c, int ** visited_places ){
   (*move)++;
    (*r) = *(valid_moves+0);
    (*c) = *(valid_moves+1);
    *(*(visited_places+(*r))+(*c)) = 1;
}

void init_thread(thread_data *args, long thread_id, int visited_count, int total_squares, int r, int c, int** visited_places, int num_moves, int move, int squares_visited, int** valid_moves, int i ){
    //allocate stuff and copy data to thread
    int** thread_valid_moves = (int**)calloc(8*2, sizeof(int*));
    int** thread_visited_places = (int**)calloc(m, sizeof(int*));
    for(int i=0;i!=m;i++){
        *(thread_visited_places+i) = (int*) calloc(n, sizeof(int));
        for(int j=0;j!=n;j++){
            *(*(thread_visited_places+i)+j) = *(*(visited_places+i)+j);
        }
    }
    for(int i=0;i!=8*2;i++){
        *(thread_valid_moves+i) = (int*) calloc(2, sizeof(int));
        *(*(thread_valid_moves+i)+0) = *(*(valid_moves+i)+0);
        *(*(thread_valid_moves+i)+1) = *(*(valid_moves+i)+1);
    }
 
    if(thread_id){
        step(&move, *(thread_valid_moves+i), &r, &c, thread_visited_places);
    }

   

    args->thread_id = next_thread_number;
    pthread_mutex_lock(&mutex);
    next_thread_number ++;
     pthread_mutex_unlock(&mutex);
    args->squares_visited = squares_visited;
    args->visited_places = thread_visited_places;
    args->r = r;
    args->c = c;
    args->valid_moves   = thread_valid_moves;
    args->visited_count = visited_count;
    args->total_squares = total_squares;
    args->move  = move;
}

void create_threads(long thread_id, long* thread_id_array, pthread_t thread_t, int num_moves, int visited_count, int total_squares,int r, int c, int** visited_places, int move, int squares_visited, int** valid_moves){
    #ifndef QUIET
        if(thread_id ==0){
            printf("MAIN: %d possible moves after move #%d; creating %d child threads...\n", num_moves, move, num_moves);
        }else{
            printf("T%lu: %d possible moves after move #%d; creating %d child threads...\n", thread_id, num_moves, move, num_moves);
        }
    #endif


    for(int i=0;i!=num_moves;i++){
        thread_data *args = (thread_data *) calloc (1, sizeof(thread_data));
        //init args to pass to thread 
        init_thread(args, next_thread_number, visited_count, total_squares, r, c, visited_places, num_moves, move, squares_visited, valid_moves, i);
        //create child threads
        pthread_mutex_lock(&mutex);
        pthread_create(&thread_t, NULL, findTour, (void *)args); 
        *(ThreadIDLookUp + next_thread_number-1) = thread_t;
        *(thread_id_array+i) = thread_t;
        pthread_mutex_unlock(&mutex);
        #ifdef NO_PARALLEL
            pthread_join(*(thread_id_array+i), NULL);
            pthread_mutex_lock(&mutex);
            #ifndef QUIET
                if(thread_id == 0){
                    //MAIN thread 
                    printf("MAIN: T%lu joined\n", thread_id_lookup(thread_t));
                }else{
                    printf("T%lu: T%lu joined\n", thread_id, thread_id_lookup(thread_t));
                }
            #endif
            pthread_mutex_unlock(&mutex);
        #endif
    }
}


int check_found_tour(long thread_id, int visited_count, int total_squares,int r, int c, int** visited_places, int** valid_moves, int num_moves, int move ){
    // print_board(r, c, visited_count, visited_places, total_squares, move);
    if(visited_count==total_squares){
			if(org_r==r && org_c==c){
				//knights closed tour detected;
				#ifndef QUIET
                    if(thread_id==0){
                        printf("MAIN: Sonny found a full knight's tour; incremented total_open_tours\n");
                    }else{
					    printf("T%lu: Sonny found a full knight's tour; incremented total_open_tours\n",thread_id);
                    }
				#endif
                total_closed_tours++;
			}else{
				//knights open tour detected;
				#ifndef QUIET
                if(thread_id==0){
                    printf("MAIN: Sonny found an open knight's tour; incremented total_open_tours\n");
                }else{
				    printf("T%lu: Sonny found an open knight's tour; incremented total_open_tours\n", thread_id);
                }
				#endif
                total_open_tours++;
			}
            return 1;
		}
        if(num_moves==0){
            #ifndef QUIET
                int b = 0;
            #endif
            if(move > max_squares){
                max_squares = move;
                #ifndef QUIET
                    b = 1;
                #endif
            }
			#ifndef QUIET
                if(thread_id==0){
                    if(b){
                        printf("MAIN: Dead end at move #%d; updated max_squares\n", move);
                    }else{
                        printf("MAIN: Dead end at move #%d\n", move);
                    }
                }else{
                    if(b){
                        printf("T%lu: Dead end at move #%d; updated max_squares\n", thread_id, move);
                    }else{
				        printf("T%lu: Dead end at move #%d\n", thread_id, move);
                    }
                }
			#endif
            return 1;
        }
        return 0;

}

void print_board(int r, int c, int visited_count, int** visited_places, int total_squares, int move){
    //print board
    printf("\npos:(%d,%d)   visited_count:%d move:%d\n", r, c, visited_count, move);
    for(int i=0;i!=n;i++){
        for(int j=0;j!=m;j++){
            printf("%d", *(*(visited_places+j)+i));
        }
        printf("\n");
    }
    printf("\n");
}

int possible_moves(int r, int c, int** visited_places, int** valid_moves, int* visited_count){
    //possible knight moves
	int* x =(int*)calloc(8, sizeof(int));
	*(x)   = -2;
	*(x+1) = -1;
	*(x+2) = 1;
	*(x+3) = 2;
	*(x+4) = 2;
	*(x+5) = 1;
	*(x+6) = -1;
	*(x+7) = -2;
	int* y = (int*)calloc(8, sizeof(int));
	*(y)   = 1;
	*(y+1) = 2;
	*(y+2) = 2;
	*(y+3) = 1;
	*(y+4) = -1;
	*(y+5) = -2;
	*(y+6) = -2;
	*(y+7) = -1;
    int num_moves = 0;
    for(int i=0;i!=8;i++){
        if(0<=r+(*(x+i)) && r+(*(x+i)) <= (m-1) && 0<=c+(*(y+i)) && c+(*(y+i))<= (n-1)){
            if(*(*(visited_places+(r+(*(x+i))))+(c+(*(y+i)))) == 0){
                *(*(valid_moves+num_moves)+0) = r+(*(x+i));
                *(*(valid_moves+num_moves)+1) = c+(*(y+i));
                num_moves++;
            }
        }
    }
    *visited_count = 0;
    for(int i=0;i!=m;i++){
        for(int j=0;j!=n;j++){
            if(*(*(visited_places+i)+j) == 1){
                (*visited_count)++;
            }
        }
    }
    free(x);
	free(y);
    return num_moves;
}

void *findTour(void *_args){
    /* Cast the arguments to the usable struct type */
    struct thread_data *args = (struct thread_data *) _args;
    long thread_id  = args->thread_id;
    int squares_visited     = args->squares_visited;
    int** visited_places    = args->visited_places;
    int r   = args->r;
    int c   = args->c;
    int** valid_moves    = args->valid_moves;
    int visited_count   = args->visited_count;
    int total_squares   = args->total_squares;
    int move    = args->move;
    int finished = 0;
    while(!finished){
        //find how many possibe moves and update the board
        int num_moves = possible_moves(r, c,visited_places, valid_moves, &visited_count);
        pthread_mutex_lock(&mutex);
        //check if dead end/tour is found
        int found = check_found_tour(thread_id, visited_count, total_squares, r, c, visited_places, valid_moves, num_moves, move );
        pthread_mutex_unlock(&mutex);
        if(num_moves==1 && !found){
            //if only one move, dont need to make a thread, just step
            step(&move, *(valid_moves+0), &r, &c, visited_places);
        }else{
            if(num_moves >1 && !found){
                long* thread_id_array = (long*)calloc(num_moves, sizeof(long));
                pthread_t thread_t = 0;
                //make thread for each move
                create_threads(thread_id, thread_id_array, thread_t, num_moves, visited_count, total_squares, r, c, visited_places, move, squares_visited, valid_moves);
                #ifndef NO_PARALLEL
                    //join threads back to thread
                    for(int i=0; i!= num_moves; i++){
                        pthread_join(*(thread_id_array+i), NULL);
                        #ifndef QUIET
                            pthread_mutex_lock(&mutex);
                            if(thread_id == 0){
                                //MAIN thread
                                printf("MAIN: T%lu joined\n", thread_id_lookup(*(thread_id_array+i)));
                                finished = 1;
                            }else{
                                printf("T%lu: T%ld joined\n", thread_id, thread_id_lookup(*(thread_id_array+i)));
                            }
                            pthread_mutex_unlock(&mutex);
                        #endif
                    }
                #endif
                free(thread_id_array);
            }
            //free stuff
            for(int i=0;i!=m;i++){
                free(*(visited_places+i));
            }
            free(visited_places);
            for(int i=0;i!=16;i++){
                free(*(valid_moves+i));
            }
            free(valid_moves);
            free(args);

            if(thread_id != 0){
                pthread_exit (NULL);
            }
            return NULL;
        }
    } 
    return NULL;
}


int simulate( int argc, char ** argv ){
    next_thread_number = 0;
    //init args
    if(argc<4){
        fprintf(stderr, "ERROR: Invalid number of arguments\nUSAGE: a.out <m> <n> <r> <c>\n");
	exit(EXIT_FAILURE);
    }
	m = atoi(*(argv + 1));
	n = atoi(*(argv + 2));
	int r = atoi(*(argv + 3));
	int c = atoi(*(argv + 4));
	org_r = r;
	org_c = c;
	if(m<2 || n <2 || r>=m || c>=n){
		fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> <r> <c>\nEXAMPLE: a.out 2 2 1 1\n");
		exit(EXIT_FAILURE);
	}
	int move = 0;
	int num_moves = 1;
	int total_squares = m * n;
	int squares_visited=0;

    //initial printing
	printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n", m, n);
	move ++;
	printf("MAIN: Sonny starts at row %d and column %d (move #%d)\n", r, c, move);
	
    //allocate stuff
    ThreadIDLookUp = (long*)calloc(MAX_THREADS, sizeof(long));
	int** valid_moves = (int**)calloc(8*2, sizeof(int*));
	int** visited_places = (int**)calloc(m, sizeof(int*));
	int visited_count = 0;
	for(int i=0;i!=m;i++){
		*(visited_places+i) = (int*) calloc(n, sizeof(int));
		for(int j=0;j!=n;j++){
			*(*(visited_places+i)+j) = 0;
		}
	}
	for(int i=0;i!=8*2;i++){
		*(valid_moves+i) = (int*) calloc(2, sizeof(int));
	}
	*(*(visited_places+r)+c) = 1;

	//start finding tours from main "thread"
    thread_data *args = (thread_data *) calloc (num_moves, sizeof(thread_data));
    // thread_data args;
    init_thread(args, next_thread_number, visited_count, total_squares, r, c, visited_places, num_moves,  move, squares_visited, valid_moves, 0);
    findTour(args);
	if(total_open_tours){
		printf("MAIN: Search complete; found %d open tours and %d closed tours\n", total_open_tours, total_closed_tours);
	}else if(max_squares == 1){
        printf("MAIN: Search complete; best solution(s) visited %d square out of %d\n", max_squares, total_squares);
    }
    else {
        if(max_squares == 1){
		    printf("MAIN: Search complete; best solution(s) visited %d squares out of %d\n", max_squares, total_squares);
        }else{
		    printf("MAIN: Search complete; best solution(s) visited %d squares out of %d\n", max_squares, total_squares);
        }
	}
	//free stuff
	for(int i=0;i!=m;i++){
		free(*(visited_places+i));
	}
	free(visited_places);
	for(int i=0;i!=16;i++){
		free(*(valid_moves+i));
	}
	free(valid_moves);
    free(ThreadIDLookUp);
    return 0;

}
