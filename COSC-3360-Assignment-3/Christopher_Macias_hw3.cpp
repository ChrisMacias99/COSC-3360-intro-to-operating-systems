
//Created by Christopher Macias using repl.it
//final draft completed on 12/2/2019





#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<unistd.h>
#include<fstream>
#include<sstream>
#include<vector>
#include<map>
#include<cmath>
#include<utility>
#include<semaphore.h>
#include <fcntl.h>
#include  <sys/ipc.h>




using namespace std;



class Frame{
  public:
  string p_id;
  string page;

};

sem_t *mySem;//initialized the semaphore that will order the processes

string *frame_storage;//I need to put this in shared memory

int shmid; //this is the Id reference_vector that will be used in the shared memory segment that all of the process will use

key_t shmkey;//we will also need this when creating the shared memory

int total_page_frames;//this is the total number of page frame_vector in main memory
int page_size;//The size of each page(in bytes)

int r;//the number of page frame_vector per process

int x;//the lookahead value for lru and opt

int min_pool_size;//this will account for the minimum pool size of 
int max_pool_size;

int p;//This is the number of processes that are to be created




vector<Frame>frame_vector;// im going to place all of these frame_vector together in this vector


int convert_to_int(string inputstring){
  return stoi(inputstring);
}




string hexToBinary(string hex_string){ //This function will turn the hexadecimal address into a binary one so I can use it to make a binary conversion into a decimal number later.
    string binary_string;
    int i = 2;
    while(hex_string[i]){
        switch(hex_string[i]){//Use switch statements to look for the given hex values up to 15, or "F"
            case '0': binary_string.append("0000"); break;
            case '1': binary_string.append("0001"); break;
            case '2': binary_string.append("0010"); break;
            case '3': binary_string.append("0011"); break;
            case '4': binary_string.append("0100"); break;
            case '5': binary_string.append("0101"); break;
            case '6': binary_string.append("0110"); break;
            case '7': binary_string.append("0111"); break;
            case '8': binary_string.append("1000"); break;
            case '9': binary_string.append("1001"); break;
            case 'A': binary_string.append("1010"); break;
            case 'B': binary_string.append("1011"); break;
            case 'C': binary_string.append("1100"); break;
            case 'D': binary_string.append("1101"); break;
            case 'E': binary_string.append("1110"); break;
            case 'F': binary_string.append("1111"); break;
            default: cout << "hexadecimal conversion not correct." << endl;
        }
        i++;
    }
    return binary_string;
}












//This function will convert a binary number to a decimal number. This is going to be used whenever we are going to compare the hexadecimal addresses with the addresses in the page table.
int binary_to_decimal(string x){
    int size = int(x.size());
    int y = 0;
    int mult = 1;
    for(int i = size - 1 ; i >= 0; i--){
        if(x[i] == '1'){
            y += mult;
        }
        mult *= 2;
    }
    return y;
}



//Im going to use this function to convert a decimal value into a binary value
string decToBinary(int n, int binary_value){
    string x;
    vector<int> binaryNum;
    binaryNum.resize(binary_value);
    for(int i = 0; i < binary_value; i++){
        binaryNum[i] = 0;
    }
    int i = 0;
    while (n > 0){
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
    // printing binary array in reverse order
    int k = binary_value - i - 1;
    for (int j = k + i - 1; j >= 0; j--){
        stringstream ss; ss << binaryNum[j];
        x.append(ss.str());
    }
    return x;
}


//This is the function that will be used to replace the pages in the frame at random
int random_page_replacement(vector<string>frame_vector, vector<string> reference_vector){
    int x=0; 
    time_t t;
    srand((unsigned)time(&t));//Generate a random seed that will be used to generate the random location that the page replacement will occur.
    for(int i = 0; i < reference_vector.size(); i++){
        int check = 0;
        for(int j = 0; j < frame_vector.size(); j++){
            if(frame_vector[j] == reference_vector[i]){
                check = 1;
                break;
            }
        }
        if(check == 1){
            continue;
        }
        int random = rand() % (frame_vector.size());
        frame_vector[random] = reference_vector[i];
        x++;
    }
    return x;
}








//This program will perform the page replacements using the least recently used algorithm.
int least_recently_used(vector<string> frame_vector, vector<string> reference_vector){
    int x = 0;
    vector<int> frame_length;
    frame_length.resize(frame_vector.size());
    for(int i = 0; i < frame_length.size(); i++){
        frame_length[i] = 9999;
    }
    for(int i = 0; i < reference_vector.size(); i++){
        int check = 0;
        int lru = -1;
        int replaceframe = 100000;
        for(int j = 0; j < frame_vector.size(); j++){ //This checks if it's part of the frame_vector currently in there
            if(frame_vector[j] == reference_vector[i]){
                check = 1;
                break;
            }
        }
        if(check == 1){
            continue;
        }

        for(int j = 0; j < frame_length.size(); j++){ //This for loop only runs if we have a page fault
            if(frame_length[j] == 9999){ //This means if you find a frame that has never been taken up before
                replaceframe = j;
                break;
            }
            else if(frame_length[j] > lru){ //This means you find a frame that has been used less recently than the one you just found
                lru = frame_length[j];
                replaceframe = j;
            }
        } //At the end of the loop I have the frame that I want to insert the current page into
        frame_vector[replaceframe] = reference_vector[i];
        frame_length[replaceframe] = 0;
        for(int i = 0; i < frame_length.size(); i++){
            frame_length[i] = frame_length[i] + 1;
        }
        x++;
    }
    return x;
}


//This function will perform the page replacements and give the appropriate number of page faults.
int first_in_first_out(vector<string> frame_vector, vector<string> reference_vector){ 
    int x = 0;
    int avail;
    for(int i = 0; i < frame_vector.size(); i++){
        frame_vector[i] = -1;
    }
    int j = 0;
    for(int i = 0; i < reference_vector.size(); i++){
        avail = 0;
        for(int k = 0; k < frame_vector.size(); k++){
            if(frame_vector[k] == reference_vector[i]){
                avail = 1;
            }
        }
        if(avail == 0){
            x++;
            frame_vector[j] = reference_vector[i];
            j = (j+1)%frame_vector.size();
            for(int k = 0; k < frame_vector.size(); k++){
            }
        }
    }
    return x;
}







int main(int argc, const char *argv[]) {
  shmkey=ftok("/",'a');

  frame_storage=(string *) shmat(shmid, 0, 0);//This will place the frame_vector into a global shared memory that can be accessed by the processes.



  mySem=sem_open("mySemid", O_CREAT | O_EXCL, 0644, 1);
  //i need to place these objects in shared memory using shmid.
  shmid=shmget(shmkey,sizeof(int),0644|IPC_CREAT);
  
  //allow the semaphore to be used by multiple processes
  sem_unlink("mySemid");

 
  ifstream inputFile;
  string TextFile="input.txt";//I hope this doesn't become a problem when the TAs put my code into the server to test.
  inputFile.open(TextFile);
  

  cout<<"Sucessfully opened file '"<<TextFile<<"' "<<endl;
  string currentline;//this string will store the line we are currently looking at in the file

  getline(inputFile,currentline);

  total_page_frames=convert_to_int(currentline);//Using my conversion function, i'm gonna place the integers into the corresponding values by going line by line.
  
  cout<<"total page frame_vector="<<total_page_frames<<endl;

  frame_vector.resize(total_page_frames);//now that we know the number of page frame_vector we have, we can alter the size of the vector to correspond to this value

  getline(inputFile,currentline);

  page_size= convert_to_int(currentline);

  cout<<"Page size= "<<page_size<<endl;

  getline(inputFile,currentline);
  
  r=convert_to_int(currentline);

  cout<<"The value of r is currently "<<r<<endl;

  getline(inputFile,currentline);

  x=convert_to_int(currentline);

  cout<<"The current value of x is "<<x<<endl;

  getline(inputFile,currentline);


  min_pool_size=convert_to_int(currentline);

  cout<<"The minimum pool size is "<<min_pool_size<<endl;

  getline(inputFile,currentline);

  max_pool_size=convert_to_int(currentline);

  cout<<"The maximum pool size is "<<max_pool_size<<endl;

  getline(inputFile,currentline);

  p=convert_to_int(currentline);

  cout<<"The number of processes in this program is "<<p<<endl;

  
 //This is where the fun begins........ after I fork, i need to make sure that the corresponding processes gather data from the correct process values and place them in the correct vector that will be compared with the frame_vector during the page replacement algorithm checks. Here we go......
  
  int pid=1;//setting the pid equal to 1 so i can confirm that the forking of the processes is successful.
  int comparison_pid;//This is going to be used by the children processes to ensure that they gather from the correct 
  int psize=0;//This integer will be used to alter the page table size later on in the program
  int process_counter=0;//This is going to be used to iterate the process data to correspond to which child is going to be using it.
  string address;

  vector<string>address_vector;//this is where i will put the addresses in their original hexadecimal form

  vector<string>integer_address_vector;//this will hold the integer representation of the addresses after i convert them.

  vector<string>process_frames_vector;//Im going to use this to hold the frame_vector that the individual processes have. in this case, the variable will be represented by "r"
  
  
  
  process_frames_vector.resize(r);
  
  //Here i'm going to set the values of all of the active frame_vector to negative 1 that way i will be able to know that they are empty and it will be easy to determine a page fault.
  for(int i=0; i<process_frames_vector.size(); i++){
    process_frames_vector[i]="-1";
    
  }

 

  

  for(int j=0;j<p;j++){//Fork child processes for the number of processes given by the input file
    
    if(pid==0){//If the pid is zero then I am done creating the child processes
      
      break;
    }
    else{
      process_counter++;
      pid=fork();//Here is where I will make the child processes.
    }
  }
   if(pid==0){//The children are going to handle reading the addresses from the file and running the page replacement algorithms.

   for(int i=0;i<process_counter-1;i++){//this for loop is going to be used to skip this line as it is not needed

   getline(inputFile,currentline);
   }

   getline(inputFile,currentline);

   string pid_string=currentline.substr(0,currentline.find(" "));//this string will be used to get the pid values in the correct place

   string psize_string=currentline.substr(currentline.find(" ")+1);//I have to create a substring to extract the psize from the line.

   pid=convert_to_int(pid_string);
   
   
   psize=convert_to_int(psize_string);
   
   
   int pc=psize+16-(psize%16);//This is going to give the correct bit size for the page table
   
   
   for(int k=0; k<p-process_counter;k++){
     getline(inputFile,currentline);
     
   } 
   
   getline(inputFile,currentline);

  
   
   string comparison_pid_string=currentline.substr
   (0,currentline.find(" "));

   
   
   comparison_pid=convert_to_int(comparison_pid_string);

   string process_address=currentline.substr(currentline.find(" ")+1);
   
   address=process_address;
   
   
   //As long as the address that is gathered from the file is not negative 1, I will continue to parse the file until a negative 1 is located.
   while(address !="-1"){
     if(comparison_pid==pid){
     address_vector.push_back(address);//Push back the address into the address vector if it isnt a negative 1. Each process will do this separately from each other.
    }
    getline(inputFile,currentline);
    string comparison_pid_string=currentline.substr(0,currentline.find(" "));

   
   //This comparison_pid will be used by the process to place the addresses into the correct address vector
   comparison_pid=convert_to_int(comparison_pid_string);   
   string process_address=currentline.substr(currentline.find(" ")+1);
   
   address=process_address;
   }

   for(int i=0; i<address_vector.size();i++){//I just want to double check that the addresses are correctly placed into the address vector. I might delete this later
     cout<<address_vector[i]<<" ";
   }

   cout<<endl;

   integer_address_vector.resize(address_vector.size());//This address vector contains the same hex addresses read into the address vector, except I am going to convert them into integers so that way it is easier to reference_vector them in the page frame_vector.

   vector<int> page_table_vector;

   page_table_vector.resize(psize);
   sem_wait(mySem);//Semaphore will wait to institute order among all of the other processes
   for(int i=0; i<address_vector.size(); i++){//This loop is where I will take the addresses from the address vector and convert them into an integer value.
     integer_address_vector[i] = to_string(binary_to_decimal(hexToBinary(address_vector[i])));
   }
   //Each Child process should cout the given page faults. I was reading that I need exit codes at the end to terminate the child processes but i'm not too sure. I will leave them out for now and will try to add them if they dont finish correctly.
    cout << "Number of Page faults for FIFO=" << first_in_first_out(process_frames_vector, integer_address_vector) <<". Process number="<<pid<< endl;

    cout << "Number of Page faults for LRU="<< least_recently_used(process_frames_vector, integer_address_vector)<<". Process number="<<pid<<endl;

    cout<<"Number of Page faults for RANDOM="<< random_page_replacement(process_frames_vector, integer_address_vector) << ". Process number="<<pid<< endl;
   sem_post(mySem);//Signal that the process is done working with the shared memory.
  }
 else if(pid!=0){//For some reason, the parent would not wait for the children to run, so i had to institute a wait so that it waits until all of the children processes complete, hence why I have the for loop.
  for(int i=0; i<p;i++){
    wait(NULL);
  }
}
return 0;
}

//This is my final draft of assignement 3. Should any problems arise with file input, I would appreciate if you would let me know. Thanks yall. Have a good winter break.


//Christopher Macias
//Peoplesoft ID: 1492467