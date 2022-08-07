#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

int **adj;
int *partitionArray;
int *ievertex;
int *colour;

mutex coarseLock;

ofstream out("output.txt");

void createPartitions(int *partitionArray,int n, int k);
void checkVertex(int **adj,int *partition,int n,int k,int vertex,int partitionNum,int *ievertex);
void printArray(int *arr,int size);
void ThFunctionCoarseGrain(int partitionNumber,vector<int> vertexInPartition,int n);
int maxFromArray(int *arr,int size);
void printColourArray(int *arr,int size);

int main()
{
    int n,k;//Number of vertices, Num of threads or partitions
    ifstream fin("input_params.txt");
    fin>>k>>n;
    int value;
    

/****create partition array ****/    
    partitionArray = new int[n];
/****create ievertex array ****/    
    ievertex = new int[n];
/****create colour array ****/    
    colour = new int[n];
    for(int i=0;i<n;i++){
        colour[i]=-1;
    }
/****create adjacency matrix and scan elements from file ****/   
    adj =new int*[n];
    for(int i=0;i<n;i++){
        adj[i]=new int[n];
    }
	for(int i=0;i<n;i++){
		fin>>value;
	}
    for(int i=0;i<n;i++){
		fin>>value;
        for(int j=0;j<n;j++){\
            fin>>value;
            adj[i][j]=value;
        }
    }
//end adjacency matrix


    createPartitions(partitionArray,n,k);
    for(int i=0;i<n;i++){
        checkVertex(adj,partitionArray,n,k,i,partitionArray[i],ievertex);
    }
/*
	for(int i=0;i<n;i++){
        printArray(adj[i],n);
    }
        cout<<endl;

    printArray(partitionArray,n);
        cout<<endl;

    printArray(ievertex,n);
*/

    vector<int> *partitionVec;//Array of vectors.//partitionVec[2] is vector which contains elements that are in partition 2
    partitionVec=new vector<int>[k];
    for(int i=0;i<n;i++){//For all vertex put them in their respective partition
        partitionVec[partitionArray[i]].push_back(i);
    }
	
    time_point<system_clock> start_time,end_time;
    duration<double> execution_time(0);
    vector<thread> v;//n threads
    start_time = system_clock::now();
    for(int i=0;i<k;i++){
		v.push_back(thread(ThFunctionCoarseGrain,i,std::ref(partitionVec[i]),n));
    }
    for(auto& t : v) t.join();//iterate over the vector of threads auto for vector_iterator
    end_time = system_clock::now();

	out<<"Coarse Lock\nNo. of colours used: "<<maxFromArray(colour,n)+1<<endl;
    execution_time=1000*(end_time - start_time);
    out<<"Time taken by the algorithm using: "<<execution_time.count() <<" Millisecond"<<endl;
    out<<"Colours:\n";
    printColourArray(colour,n);


/****delete the adjacency matrix****/
    for(int i=0;i<n;i++){
        delete [] adj[i];
    }
    delete [] adj;
/****delete the partition array****/
    delete [] partitionArray;
    delete [] ievertex;

    out.close();
    return 0;
}





void createPartitions(int *partitionArray,int n, int k){
    for(int i=0;i<n;i++){
        partitionArray[i]=rand()%k;
    }
}
void printArray(int *arr,int size){
    for(int i=0;i<size;i++){
        cout<<arr[i]<<" ";
    }
    cout<<endl;
}
void printColourArray(int *arr,int size){
    for(int i=0;i<size-1;i++){
        out<<"v"<<i+1<<" - "<<arr[i]<<", ";
    }
    out<<"v"<<size<<" - "<<arr[size-1]<<endl;
}

int maxFromArray(int *arr,int size){
    int i;
    // Initialize maximum element
    int max = arr[0];
 
    // Traverse array elements
    // from second and compare
    // every element with current max
    for (i = 1; i < size; i++)
        if (arr[i] > max)
            max = arr[i];
 
    return max;
}
vector<int> getNeighbours(int **adj,int n,int vertex){
    vector<int> neighbour;
    for(int i=0;i<n;i++){
        if(adj[vertex][i]==1){
            neighbour.push_back(i);
        }
    }
    return neighbour;
}
void checkVertex(int **adj,int *partition,int n,int k,int vertex,int partitionNum,int *ievertex){
    vector<int> neighbour=getNeighbours(adj,n,vertex);

    for(auto &t: neighbour){
        if(partition[t]!=partitionNum){
            ievertex[vertex]=1; //external
            return;
        }
    }
    ievertex[vertex]=0; //internal
}

void ThFunctionCoarseGrain(int partitionNumber,vector<int> vertexInPartition,int n){
	for(auto &currentVertex: vertexInPartition){
        if(ievertex[currentVertex]==0){//currentVertex is internal vertex
            //colour with minimum available currently
            vector<int> neighbour=getNeighbours(adj,n,currentVertex);
            int *local_availables;
            local_availables=new int[neighbour.size()+1];
            for(int i=0;i<neighbour.size()+1;i++){
                local_availables[i]=1;//ALL 0 to number_of_edges_from_current_vertex are available.
            }
            for(auto &neighbourVertex: neighbour){
                if(colour[neighbourVertex]==-1 || colour[neighbourVertex]>neighbour.size()){
                    continue;
                }
                local_availables[colour[neighbourVertex]]=0;//Set the local available colour for the index false since a neighbour has that colour
            }
            int theMinimumAvailable=0;
            for(theMinimumAvailable=0;theMinimumAvailable<neighbour.size()+1;theMinimumAvailable++){
                if(local_availables[theMinimumAvailable]==1){
                    break;
                }
            }
            colour[currentVertex]=theMinimumAvailable;//No sync problem.
            delete [] local_availables;
        }
        else if(ievertex[currentVertex]==1){//currentVertex is external vertex
            coarseLock.lock();
            
            //colour with minimum available currently
            vector<int> neighbour=getNeighbours(adj,n,currentVertex);
            int *local_availables;
            local_availables=new int[neighbour.size()+1];
            for(int i=0;i<neighbour.size()+1;i++){
                local_availables[i]=1;//ALL 0 to number_of_edges_from_current_vertex are available.
            }
            for(auto &neighbourVertex: neighbour){
                if(colour[neighbourVertex]==-1 || colour[neighbourVertex]>neighbour.size()){
                    continue;
                }
                local_availables[colour[neighbourVertex]]=0;//Set the local available colour for the index false since a neighbour has that colour
            }
            int theMinimumAvailable=0;
            for(theMinimumAvailable=0;theMinimumAvailable<neighbour.size()+1;theMinimumAvailable++){
                if(local_availables[theMinimumAvailable]==1){
                    break;
                }
            }
            colour[currentVertex]=theMinimumAvailable;//No sync problem.
            delete [] local_availables;

            coarseLock.unlock();
        }
    }
}


/*******CODE EXPLANATION*********/
/*
Input as adjacency matrix of size n*n ----
For vertex v declare its partition by random between 1 and k.Total vertex = n  ----
Check for all vertices whether it is internal vertex or external vertex of it's own partition and store the result in ievertex array ----
Create a colour array of size n. ----

Create vector for each partition. This vector consists which vertices belong to partition. EX:- partition 4 contains <2,8,6> vertices
Pass the vector into thread. EX:- partition 4 contains <2,8,6> vertices so thread_id==4 will get <2,8,6> as input

Each thread will work as follows :-
    Grab a vertex from vector.
    check whether it is internal vertex or external vertex.(Compare the value in ievertex array)
    if internal:
        colour
    if external:
        check with other threads and then colour

*/



