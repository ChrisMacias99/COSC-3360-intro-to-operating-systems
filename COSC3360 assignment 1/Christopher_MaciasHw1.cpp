/*Note for TA: as of 9/30/2019, I removed all of my arbitrary cout statements from my program in order to reduce clutter and enhence readability. The program writes the output to the output
file so it will not appear in the console. I apologize for the inconvienience.*/
//  main.cpp
//  COSC3360 Assignment 1
//  Christopher Macias 
//  1492467
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <cstring>
using namespace std;

/*This container will store the values for the variable (a,b,c,d) along with an arbitrary null character to signify that the process hasnt been interacted with. Along side that it
will include a data entry to store the operator and a vector to store the correct operands. */
class Process{ 
public:
    string variable;
    char value = 'N';
    string op;
    vector<char> operand;
};

string getString(char x){ //I may not have to use this function, but i would like to leave this definition here until i find a reputable work around.
    string s(1,x);
    return s;
    
}
/*For the goal of this assignment, we are to take in given input variables, each containing a binary true or false value, as well as 
vertexes with different machine language operation perform the assigned operations on the corresponding binary values. To do this, we are to create child processes
for each input variable and vertex using the unix fork(), pipe(), and execv() commands. We will be taking in the inputs from a given input file and we will write out the finished values into
an output file.*/

int main(int argc, const char * argv[]) {
       vector<char> input_variable;
       vector<char> truth_variable;
       vector<string> myOperation;
       vector<string> vertex_vec;
    vector<Process> process_vec;
    int count = 0;
       ifstream inFile;
    ofstream outFile("output.txt");
    
       inFile.open(argv[1]);
       
       if(inFile.fail()){
           cerr << "File could not be opened" << endl;
           exit(-1);
       }
       else{
           string word;
           char variable;
           inFile >> word; //This will take the input varable from the input file
           while(inFile >> variable){ //This will read in all of the input variables and store them in the vector, input_variable
               if(variable == ','){
                   continue;
               }
               if(variable == ';'){
                   break;
               }
               input_variable.push_back(variable);
               count++;
           }
           while(inFile >> variable){ //This will push the truth values to the truth_variable vector as well as ensure that the input_variable indexes are correct and conform with the truth values.
               if(variable == ';'){
                   break;
               }
               truth_variable.push_back(variable);
           }
           inFile >> word; //The program will now read in the operations and store them into the vector, vertex_vec
           while(inFile >> word){ //Use this to assign all vertex_vec to the vertex_vec vector, and all of their respective operations in the correct index for the operation vector
               if(word == "=" ){
                   continue;
               }
               else if(word == "OR,"){
                   myOperation.push_back("OR");
               }
               else if(word == "NOT,"){
                   myOperation.push_back("NOT");
               }
               else if(word == "AND,"){
                   myOperation.push_back("AND");
               }
               else if(word == "IMPLY,"){
                   myOperation.push_back("IMPLY");
               }
               else if(word == "OR;"){
                   myOperation.push_back("OR");
                   break;
               }
               else if(word == "NOT;"){
                   myOperation.push_back("NOT");
                   break;
               }
               else if(word == "AND;"){
                   myOperation.push_back("AND");
                   break;
               }
               else if(word == "IMPLY;"){
                   myOperation.push_back("IMPLY");
                   break;
               }
               else{
                   vertex_vec.push_back(word);
                   count++;
               }
           }
           bool exit_check = true;
           while(exit_check){
               inFile >> word;
               char checker[100]; //we will use this array to store the write function that is located at the end of the input file.
               for(int i = 0; i < word.size(); i++){
                   checker[i] = word[i];
               }
               if(checker[0] == 'w' && checker[1] == 'r'){ //If the assigned character 'w' is reached, then the program will proceed to write the answer to a created output file and terminate.
                   exit_check = false;
                   //We need to know exactly what is in this write function
                   vector<string> write;
                   int count = 6; //This will keep track of the location where we are at while parsing through the write function.
                   while(true){
                       if(checker[count] == ')'){ //We have reached the end of the 'write()' function.
                           break;
                       }
                       else if(checker[count] == ','){ //Continue to parse through the 'write()' function.
                           count++;
                           continue;
                       }
                       else if(checker[count] == 'v'){ //Push the vertex into a character vector
                           count++;
                           char vertex_vec[3] = {'v', checker[count]};
                           string s(vertex_vec);
                           write.push_back(s);
                       }
                       else{
                           string s(1,checker[count]);
                           write.push_back(s);
                       }
                       count++;
                   }
                   for(int i = 0; i < process_vec.size(); i++){ //This will make sure that all of the process_vec have ran correctly.
                       if(process_vec[i].value == 'N'){ //Should the character of this process be the null character we assigned in the process container, then we will begin perfroming the operation checks.
                           if(process_vec[i].op == "OR"){
                               //Run OR
                               for(int j = 0; j < process_vec[i].operand.size(); j++){
                                   if(process_vec[i].operand[j] == 'T'){
                                       process_vec[i].value = 'T';
                                       break;
                                   }
                                   else if(j == process_vec[i].operand.size() - 1){
                                       process_vec[i].value = 'F';
                                   }
                               }
                           }
                           else if(process_vec[i].op == "AND"){
                               //Run AND
                               for(int j = 0; j < process_vec[i].operand.size(); j++){
                                   if(process_vec[i].operand[j] == 'F'){
                                       process_vec[i].value = 'F';
                                       break;
                                   }
                                   else if(j == process_vec[i].operand.size() - 1){
                                       process_vec[i].value = 'T';
                                   }
                               }
                           }
                           else if(process_vec[i].op == "IMPLY"){
                               if(process_vec[i].operand.size() == 2){
                                   //Run the operands
                                   if(process_vec[i].operand[0] == 'F' || process_vec[i].operand[1] == 'T'){
                                       process_vec[i].value = 'T';
                                   }
                                   else{
                                       process_vec[i].value = 'F';
                                   }
                               }
                           }
                       }
                   }
                   //Now we need to make sure if its in the write vector, there is a process for it| To finish, we need to double check that all of the vectors that are located in write
                   for(int i = 0; i < write.size(); i++){
                       for(int j = 0; j < process_vec.size(); j++){
                           if(process_vec[j].variable == write[i]){ //This means that it found the process
                               outFile << process_vec[j].variable << " = " << process_vec[j].value << endl;
                               break;
                           }
                           else if(j == process_vec.size() - 1){ //The process could not be found
                               for(int k = 0; k < input_variable.size(); k++){
                                   string s(1, input_variable[k]);
                                   if(write[i] == s){
                                       outFile << write[i] << " = " << truth_variable[k] << endl;
                                       break;
                                   }
                               }
                           }
                       }
                   }
                   
                   break;
               }
               char test[2];
               if(process_vec.size() == 0){
                   Process inVariable;
                   inVariable.variable = word;
                   for(int i = 0; i< input_variable[i]; i++){
                           string x(1, input_variable[i]);
                           if(inVariable.variable == x){
                               inVariable.value = truth_variable[i];
                               break;
                           }
                       }
                   process_vec.push_back(inVariable);
                   test[0] = process_vec[0].value;
               }
               for(int i = 0; i < process_vec.size(); i++){ /*At this stage, the program will identify the current item in 'word' and check if the operations have been performed on it. 
															If this isn't the case, then the operations checks will commence on the current item.*/
                   if(word == process_vec[i].variable){
                       if(process_vec[i].value == 'N'){ //This runs the vertex_vec at the left
                           //Run the respective process
                           if(process_vec[i].op == "OR"){
                               //Run OR
                               for(int j = 0; j < process_vec[i].operand.size(); j++){
                                   if(process_vec[i].operand[j] == 'T'){
                                       process_vec[i].value = 'T';
                                       break;
                                   }
                                   else if(j == process_vec[i].operand.size() - 1){
                                       process_vec[i].value = 'F';
                                   }
                               }
                           }
                           else if(process_vec[i].op == "AND"){
                               //Run AND
                               for(int j = 0; j < process_vec[i].operand.size(); j++){
                                   if(process_vec[i].operand[j] == 'F'){
                                       process_vec[i].value = 'F';
                                       break;
                                   }
                                   else if(j == process_vec[i].operand.size() - 1){
                                       process_vec[i].value = 'T';
                                   }
                               }
                           }
                           else if(process_vec[i].op == "IMPLY"){
                               if(process_vec[i].operand.size() == 2){
                                   //Run the given operands
                                   //Ex. is T IMPLY F, this means NOT(T) OR F
                                   if(process_vec[i].operand[0] == 'F' || process_vec[i].operand[1] == 'T'){
                                       process_vec[i].value = 'T';
                                   }
                                   else{
                                       process_vec[i].value = 'F';
                                   }
                               }
                           }
                           
                       }
                       test[0] = process_vec[i].value;
                       break;
                   }
                   else if(i == process_vec.size() - 1){//This means its an input_variable
                           Process inVariable;
                           inVariable.variable = word;
                           for(int i = 0; i < input_variable[i]; i++){
                               string x(1, input_variable[i]);
                               if(inVariable.variable == x){
                                   inVariable.value = truth_variable[i];
                                   break;
                               }
                           }
                       process_vec.push_back(inVariable);
                       test[0] = process_vec.end()->value;
                   }
               }
               int fd[2];
               pipe(fd);
               pid_t a = fork(); //create a new child process
               if (a < 0){
                   cerr << "The fork went wrong" << endl;
                   exit(-1);
               }
               else if(a == 0){
                   close(fd[1]);
                   inFile >> word;
                   if( word == "->"){ //This is vertex to vertex,with the given syntax 'v0 -> v1;'
                       inFile >> word;
                       word.erase(2,1);
                       //Before finishing, we need to check if the current item in word is already registered as a new child process or not.
                       for(int i = 0; i < process_vec.size(); i++){
                           if(word == process_vec[i].variable){ //This means its an already made process
                               char test[2];
                               read(fd[0],test,10);
                               process_vec[i].operand.push_back(test[0]);
                           }
                           else if(i == process_vec.size() - 1){
                               for(int i = 0; i < vertex_vec.size(); i++){
                                   if(word == vertex_vec[i]){
                                       Process vert;
                                       vert.variable = word;
                                       vert.op = myOperation[i];
                                       char test[2];
                                       read(fd[0],test,10);
                                       vert.operand.push_back(test[0]);
                                       if(vert.op == "NOT"){ //Once we add in the operand, and we find out that the op is NOT, we will execute immediately
                                           if(vert.operand[0] == 'T'){//logical 'NOT' will change the value to the reverse.
                                               vert.value = 'F';
                                           }
                                           else if(vert.operand[0] == 'F'){
                                               vert.value = 'T';
                                           }
                                       }
                                       process_vec.push_back(vert);
                                       break;
                                   }
                               }
                               break;
                           }
                       }
                       //Now we read the vertex_vec and make a new child process
                       
                   }
                   else { //EX: This line can be read as 'a ->v0;' in the input file. At this segment, we must get rid of the '->' symbol to ensure that the processes are recorded correctly.
                       word.erase(0,2);
                       word.erase(2,1);
                       //At this point, we read the vertex_vec we are working with
                       
                       for(int i = 0; i < process_vec.size(); i++){
                           if(word == process_vec[i].variable){ //This means its an already made process
                               char test[2];
                               read(fd[0],test,10);
                               process_vec[i].operand.push_back(test[0]);
                           }
                           else if(i == process_vec.size() - 1){
                               for(int i = 0; i < vertex_vec.size(); i++){
                                   if(word == vertex_vec[i]){
                                       Process vert;
                                       vert.variable = word;
                                       vert.op = myOperation[i];
                                       char test[2];
                                       read(fd[0],test,10);
                                       vert.operand.push_back(test[0]);
                                       if(vert.op == "NOT"){ //Once we add in the operand, and we find out that the op is NOT, we will execute immediately
                                           if(vert.operand[0] == 'T'){
                                               vert.value = 'F';
                                           }
                                           else if(vert.operand[0] == 'F'){
                                               vert.value = 'T';
                                           }
                                       }
                                       process_vec.push_back(vert);
                                       break;
                                   }
                               }
                               break;
                           }
                       }
                       
                   }
                   }
               else{ //Parent Process
                   close(fd[0]);//Close the read end of the pipe.
                   write(fd[1], test, strlen(test));
                   break;
               }
           }
           outFile.close();
           inFile.close();
       }
    return 0;
	/*Note for TA: as of 9/30/2019, I removed all of my arbitrary cout statements from my program in order to reduce clutter and enhence readability. The program writes the output to the output 
	file so it will not appear in the console. I apologize for the inconvienience.*/
}
