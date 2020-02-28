
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<sys/wait.h>
#include<pthread.h>
#include <sys/mman.h>
#include<semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <fcntl.h>

using namespace std;



int m=0; //indicates the number of resources in this simulation
int n=0; // indicates the number of processes in this simulation

// i want to use this whenever we parse the line and are trying to extract the integer to assign a given variable. This will really only be used to extract the number of processes and resources.
int Integer_converter(string inputString) 
{
	return stoi(inputString);			
}

//this function will be used to get the max resources from the "max[n,m]="string.
int GetMaxResourcePerProcessorValue(string inputString)
{
	//	Find the first '=' sign which indicates that the desired value is after it
	int pos = inputString.find("=");

	//	Create a sub string for everything after the '=' sign
	string intSubString = inputString.substr(pos + 1);

	return stoi(intSubString);
}

//this function parses the input line to extract the number of available resources given by taking the integer value after the "=" sign and converting it to a usable integer.
int GetAvailableResources(string resourceString){
  string c=resourceString;
      string ava=c.substr(c.find("=")+1);
      int x=stoi(ava);
      return x;
}

sem_t *MySem;//create the semaphore

static int *available_resource_vector; // vector that will store all of the resource nodes that I create.

static int *time_elapsed;//this will keep track of how much time has been running for all processes


int remaining_processes;//I forgot why i had this but i am afraid to delete it until i am positive that I do not use this









struct Process{ //structure for the process
  int p_id;
  int deadline_miss_count=0;
  bool dead_miss;
  int deadline; //This should be the same as deadline at first
  int relative_deadline;
  int computation_time;
  int relative_computation_time;
  
  int instruction_vector_position=0;//counter for the position of the string vector
  vector<int>allocated_resources;
  vector<int>max_resources;
  vector<int>resource_need;
  vector<string>instructions;


 //this function should take in the vector of process instructions that I made and parse through them, making the correct function calls when neccessary to begin the various methods.
  void GetInstructions(vector<string>instructions){
    relative_deadline=deadline-*time_elapsed;
    
    
    cout<<"sucessfully running GetInstructions method...."<<endl<<endl;//the get instructions will now work
    
    
    
      
     cout<<"The current string you are looking at is: "<<instructions[instruction_vector_position]<<endl;//this line just tells you how far it has indexed in my vector of strings
    
    //on all of the finds, i found a method to determine if a given string is located at that current index. If it is, then i can run the appropriate function
     if(instructions[instruction_vector_position].find("request(")!=string::npos){

      cout<<"The current instruction contains 'request'. The request function will now commence....... "<<endl;
      

      Request(instructions[instruction_vector_position]);
      
    }
     else if(instructions[instruction_vector_position].find("useresources(")!=string::npos){
      cout<<"The current instruction contains 'useresources'. The request function will now commence....... "<<endl;
      
      UseResources(instructions[instruction_vector_position]);
      

    }
     else if(instructions[instruction_vector_position].find("release(")!=string::npos){
      cout<<"The current instruction contains 'release'. The release function will now commence."<<endl;
      
      Release(instructions[instruction_vector_position]);
      
      
    }
     else if(instructions[instruction_vector_position].find("calculate(")!=string::npos){
      cout<<"The current instruction contains 'calculate'. The calculate function will now commence."<<endl;

      Calculate(instructions[instruction_vector_position]);
      
     
    }
    else{
      cout<<"Error: instruction was not read correcly."<<endl;
    }

    if(instruction_vector_position==instructions.size()){
      cout<<"The instructions for the given set of instructions in vector have finished"<<endl<<endl;
      

    }
    else{
      
    }


    if(*time_elapsed>deadline){
      cout<<"Deadline missed. Instituting a deadline miss instruction"<<endl;
      deadline_miss_count++;
      dead_miss=true;


    }
    else{
     cout<<"Deadline still on time so far"<<endl;
    }


    
    
    instruction_vector_position++;

     
    
    
    
  }


  //this method is where we will perform the banker's algorithm check and the various other checks.
  void Request(string message){
    vector<int>requested_resource_values;//i will store the integer values for the request in this vector
    string sub_string=message.substr(message.find("(")+1);//substring will begin with the first character in the statement.

    
    for(int i=0; i<sub_string.size();i++){
     int x=(int)sub_string[i]-48;
     requested_resource_values.push_back(x);
     i++;
     if(sub_string[i]==')'){
       break;

     }
     
    }
    cout<<"preparing the banker's algorithm check......."<<endl;
    //preparing the banker's algorithm
    for(int i=0; i<requested_resource_values.size();i++){
      cout<<"requested values of resource "<< i+1<<" =" <<requested_resource_values[i]<<endl<<endl;
      if(requested_resource_values[i]>resource_need[i]){//this will ensure that the request can never exceed the max for any given process
        cout<<"error: the requested values exceeds the maximum need for the given process"<<endl;

      }
      else if(requested_resource_values[i]>available_resource_vector[i]){
        cout<<"The current process must wait. Now instituting a wait........"<<endl;
        //sem_post(MySem);
        //sem_wait(MySem);
      }
      else{
        cout<<"The request can now be granted......."<<endl<<endl;
        //sem_wait(MySem);//im not sure if this is where the semaphore goes
        available_resource_vector[i]=available_resource_vector[i]-requested_resource_values[i];
        allocated_resources[i]=allocated_resources[i]+requested_resource_values[i];
        resource_need[i]=resource_need[i]-requested_resource_values[i];
        //sem_post(MySem);

      }
    }

    relative_deadline--;
    relative_computation_time--;
    *time_elapsed+=1;

    cout<<"current compuation time remaining= "<<relative_computation_time<<endl;

    cout<<"Current time elapsed = "<<*time_elapsed<<endl;
    

    
    

  

  }
  
  
  
  //this method will simulate a calculation that takes x units of time and decrement the computational time.
  void Calculate(string message){
    cout<<"Successfully running the calculate method...."<<endl;
    string sub_string=message.substr(message.find("(")+1);
    int x=Integer_converter(sub_string);
   
   relative_computation_time=relative_computation_time-x;
   *time_elapsed+=x;


   cout<<"current compuation time remaining= "<<relative_computation_time<<endl;

    cout<<"Current time elapsed = "<<*time_elapsed<<endl;


  }


  //this method is going to simulate the time it takes to utilize the allocated resources to perform a process task.
  void UseResources(string message){
    string sub_string=message.substr(message.find("(")+1);
    int y=Integer_converter(sub_string);
    relative_computation_time=relative_computation_time-y;
    *time_elapsed+=y;


    cout<<"current compuation time remaining= "<<relative_computation_time<<endl;

    cout<<"Current time elapsed = "<<*time_elapsed<<endl;



  }

 //this method will release the given resources and but them back into the available array
  void Release(string message){
    cout<<"successfully running the release method...."<<endl<<endl;
    vector<int>released_ints;//i will store the integer values for the request in this vector
    string sub_string=message.substr(message.find("(")+1);//substring will begin with the first character in the statement.
    for(int i=0; i<sub_string.size();i++){
     int x=(int)sub_string[i]-48;
     released_ints.push_back(x);
     i++;
     if(sub_string[i]==';'){
       break;

     }
    }

    for(int i=0;i<allocated_resources.size(); i++){
    if(released_ints[i]>allocated_resources[i]){
      cout<<"Error: The amount of resources you are tying to release exceeds the current amount allocated"<<endl;
    }
    //sem_wait(MySem);
    allocated_resources[i]=allocated_resources[i]-released_ints[i];
    available_resource_vector[i]=available_resource_vector[i]+released_ints[i];
    //sem_post(MySem);
    }
    
    relative_computation_time--;
    *time_elapsed+=1;

    cout<<"current computation time remaining= "<<relative_computation_time<<endl;

    cout<<"Current time elapsed = "<<*time_elapsed<<endl;



  

  }

  
};


vector<Process>process_vector;//This array will store all of the process nodes that I create.


//this method will take in all of the inputs from the input file set up by argv[1] and will assign all of the appropriate values.
void ReadFromFile(string inputFileName)
{
	//	input the string argument of the input file
	fstream inputFile;

  inputFile.open(inputFileName);

	//	Evaluate the input file
	if (inputFile.is_open())
	{
		cout << "\n" << "Opened file: " << inputFileName << "\n\n";

		//	Get the first line
		string currentLine;

		//	Find & Assign the amount of resources
		getline(inputFile, currentLine);
		m = Integer_converter(currentLine);
		
		//	Initialize size of resources array & avaliable array
		available_resource_vector= new int(m);
		//available = new int[n];
		cout << "Total Resources: " << m << endl;

		//	Find & Assign the amount of processes
		getline(inputFile, currentLine);
		n = Integer_converter(currentLine);
		
		//resize the process vector to account for the total number of processes
		process_vector.resize(n);
		
		cout << "Total Processes: " << n << "\n\n";

		//	Determine the ID and amount of resources each resource has
		for (int i = 0; i < m; i++)
		{
			getline(inputFile, currentLine);
    
			//	Add the acquired integer into the resource vector for the given available resource
			
			available_resource_vector[i] = GetAvailableResources(currentLine);
			
			cout << "Resource " << i + 1 << " has " << available_resource_vector[i] << " amount of resources available." << endl;
		}

		cout << endl;
    	//	Skip a line for neatness

		//	Find & Assign the size of resource related parameters for the process
		for (int i = 0; i < n; i++)
		{
			process_vector[i].allocated_resources.resize(m);
			process_vector[i].max_resources.resize(m);
			process_vector[i].resource_need.resize(m);
		}
		//	Loop through the each process and assign the value of the max value processor can demand from each resource
		for (int i = 0; i < n; i++)
		{
			cout << "Max resources that Process " << i + 1 << " can demand from:" << endl;
			
			for (int j = 0; j < m; j++)
			{
				//	Get new line and find value in string
				getline(inputFile, currentLine);
        process_vector[i].allocated_resources[j]=0;
				process_vector[i].max_resources[j] = GetMaxResourcePerProcessorValue(currentLine);
				process_vector[i].resource_need[j] = process_vector[i].max_resources[j];

        
				
				//	Display result
				cout << " Resource " << j + 1 << ": " << process_vector[i].resource_need[j] << endl;
				
			}
		}


		cout << endl;	//	New line to help read stuff

		//	Loop through each process and cache their parameters
		for (int i = 0; i < n; i++)
		{
			//	This will ignore the line that declares the process we are about to gather its instructions from e.g "process_n".
			while (true)
			{
				getline(inputFile, currentLine);
				if (currentLine.find("process_") != string::npos)
					break;
			}
			
			cout << "Fetching parameters for " << currentLine << "..." << endl;

			//	ID
			process_vector[i].p_id = i + 1;

			//	Deadline
			getline(inputFile, currentLine);
			process_vector[i].deadline =Integer_converter(currentLine);
			cout << "Process " << i+1 << " deadline: " << process_vector[i].deadline << endl;
      process_vector[i].relative_deadline=process_vector[i].deadline;
			
			//	Computation time
			getline(inputFile, currentLine);
			process_vector[i].computation_time = Integer_converter(currentLine);
			cout << "Process " << i+1 << " computation time: " << process_vector[i].computation_time << endl;
      process_vector[i].relative_computation_time=process_vector[i].computation_time;

			//	Calculate & Assign the amount of instructions for this process
			int instructionAmount = 0;			
			streampos originalPos = inputFile.tellg();		//	keep track of where the beginning of this line is
			while (true)
			{
				getline(inputFile, currentLine);

				//	Break loop if a "end" line is found & assign the length of instructions array
				if (currentLine.find("end") != string::npos)
				{
					process_vector[i].instructions.resize(instructionAmount);
					inputFile.seekg(originalPos, ios::beg);			//	Set the getline back to the original position
					break;
				}
				instructionAmount++;
			}

			cout << "Process " << i+1 << " instructions:" << endl;

			//	Loop through instructions and cache them into process string array
			for (int j = 0; j < instructionAmount; j++)
			{
				getline(inputFile, currentLine);
				process_vector[i].instructions[j] = currentLine;

				//	increment the total amount of instructions
				//possibly not needed===> instructionsToProcess++;

				cout << " " << j+1 << ") " << process_vector[i].instructions[j] << endl;
			}

			cout << endl;	//	New line that separates the output statements to keep the command window open.
		}

		inputFile.close();
	}
	else
	{
		cout << "ERROR: invalid file input or file not found." << endl;
		exit(0);
	}
}









int main(int argc, const char * argv[]){
 string file_to_open="input.txt";
 MySem= sem_open("MySem1",O_CREAT | O_EXCL, 0644,1);//initializing the semaphore
 sem_unlink("Mysem1");
 //these bottom two lines will put both the array of available resources and the elapsed time into the shared memory space.
 time_elapsed= (int*)mmap(NULL, sizeof *time_elapsed, PROT_READ | PROT_WRITE, 
 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 available_resource_vector=(int*)mmap(NULL, sizeof *available_resource_vector, PROT_READ| PROT_WRITE, MAP_SHARED| MAP_ANONYMOUS, -1, 0);

 *time_elapsed=0;//initializing the shared time_elapsed value

 ReadFromFile(file_to_open);
 //I need to fork for the number of processes we have:
 int pid;
 int assignment=0;

 

 for(int i=0; i<n; i++){//I make the child processes here
   pid=fork();
   cout<<"child process"<<endl;
   if(pid==0){

     cout<<"Child process running....."<<endl;
     cout<<"Child processes have been created..... We reached the GetInstructions() call"<<endl;
     //this part will run while there is still computation time left for a process
    while(process_vector[assignment].relative_computation_time!=0){
      process_vector[assignment].GetInstructions(process_vector[assignment].instructions);
      for(int i=0; i<m; i++){
        cout<<"available for resource "<<i<<": "<<available_resource_vector[i]<<" "<<endl;
      }
      cout<<" ";
      for(int i=0; i<process_vector[assignment].allocated_resources.size();i++){
        cout<<"allocated resources "<<i<<": "<<process_vector[assignment].allocated_resources[i]<<endl;
      }
    
    }
    if(process_vector[assignment].dead_miss){
      cout<<"Process "<<assignment+1<<" deadline misses ="<<process_vector[assignment].deadline_miss_count<<endl;
    }
    else{
      cout<<"Process "<<assignment+1<<" has completed without missing  a deadline"<<endl;
    }

    break;
   }
    else{
      assignment++;
  

    }
     wait(NULL);//This is where the parent process will wait for all running child processes to finish 

  }
 
 
 return 0;
 
 
 
}