#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <cstring>
#include <list>
#include <array>

using namespace std;
long TIME=0; //the global time, it is the time in the output file 
long QUANTUM=100; //quantum can be changed
int howmanyprocesses=0; //it is the track of the processes we are dealing with
int completed=0; //if a process is completed we will increment this by one
long TIME_Printer1=0;
long TIME_Printer0=0;
long TIME_Harddrive=0;
int flag=0,flag1=0;
struct Block{
    int blocknum=-1;
    int last_used=0;
};

struct Process{
    int id;
    string name;
    string code_file;
    long arrival_time;
    int last_executed_line;

};

//A Process includes its id, name, codefile,arrival time and last executed line. ID, Name, codefile and arrival time is taken from definition file, last executed line is
//related with the content of codefiles, it is changed during the scheduling. 
queue<Process> Processes; //the queue of all processes, arrivals
queue<Process> Ready_Queue; //this is the queue of ready processes
queue<Process> Printer_0_Wait_Queue; //this is the queue of the waiting processes for printer 0
queue<Process> Printer_1_Wait_Queue; //this is the queue of the waiting processes for printer 1
queue<Process> Hard_Drive_Wait_Queue;//this is the queue of waiting processes for hard-drive
queue<long> Printer_0_Wait_Queue_Time;
queue<long> Printer_1_Wait_Queue_Time;
queue<long> Hard_Drive_Wait_Queue_Time;
array<Block,2> cache; //LRU cache with 2 blocks
//This function outputs the TIME and ready queue. When a change occurs in the ready queue, we are calling this function.

int cache_find(int value){
    for(int i=0;i<2;i++){
        if(cache[i].blocknum==value){
            cache[i].last_used=1;
            cache[1-i].last_used=0;
            return value;
        }
    }
    return -1;
}
bool cache_put(int value){
        for(int i=0;i<2;i++){
            if(cache[i].blocknum == -1){
                cache[i].blocknum=value;
                cache[i].last_used=1;
                cache[1-i].last_used=0;
                return true;
            }
        }
        for(int i=0;i<2;i++){
            if(cache[i].last_used==1){
                cache[1-i].blocknum=value;
                cache[i].last_used=0;
                cache[1-i].last_used=1;
                return false;
            }
        }
}
void update_outputfile(ofstream* out){
    string tire = "-";
    string ready_queue;
    if(!Ready_Queue.empty()){
    queue<Process> copy_queue = Ready_Queue;
    ready_queue= copy_queue.front().name;
    copy_queue.pop();
    while(!copy_queue.empty()){
        ready_queue.append(tire);
        ready_queue.append(copy_queue.front().name);
        copy_queue.pop();
    }
    *out<<TIME<<"::HEAD-"<<ready_queue<<"-TAIL"<<endl;}
    else{
        *out<<TIME<<"::HEAD--TAIL"<<endl;
    }
}
void update_outputfile_harddrive(ofstream* out,long time){
    string tire = "-";
    string ready_queue;
    if(!Hard_Drive_Wait_Queue.empty()){
        queue<Process> copy_queue = Hard_Drive_Wait_Queue;
        ready_queue= copy_queue.front().name;
        copy_queue.pop();
        while(!copy_queue.empty()){
            ready_queue.append(tire);
            ready_queue.append(copy_queue.front().name);
            copy_queue.pop();
        }
        *out<<time<<"::HEAD-"<<ready_queue<<"-TAIL"<<endl;}
    else{
        *out<<time<<"::HEAD--TAIL"<<endl;
    }
}
void update_outputfile_printer0(ofstream* out,long time){
    string tire = "-";
    string ready_queue;
    if(!Printer_0_Wait_Queue.empty()){
        queue<Process> copy_queue = Printer_0_Wait_Queue;
        ready_queue= copy_queue.front().name;
        copy_queue.pop();
        while(!copy_queue.empty()){
            ready_queue.append(tire);
            ready_queue.append(copy_queue.front().name);
            copy_queue.pop();
        }
        *out<<time<<"::HEAD-"<<ready_queue<<"-TAIL"<<endl;}
    else{
        *out<<time<<"::HEAD--TAIL"<<endl;
    }
}
void update_outputfile_printer1(ofstream* out,long time){
    string tire = "-";
    string ready_queue;
    if(!Printer_1_Wait_Queue.empty()){
        queue<Process> copy_queue = Printer_1_Wait_Queue;
        ready_queue= copy_queue.front().name;
        copy_queue.pop();
        while(!copy_queue.empty()){
            ready_queue.append(tire);
            ready_queue.append(copy_queue.front().name);
            copy_queue.pop();
        }
        *out<<time<<"::HEAD-"<<ready_queue<<"-TAIL"<<endl;}
    else{
        *out<<time<<"::HEAD--TAIL"<<endl;
    }
}
//This function takes the file and the line number as number and goes to that line of file. I took this function from stackoverflow.
fstream& GotoLine(fstream& file, unsigned int num){
    file.seekg(ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(numeric_limits<streamsize>::max(),'\n');
    }
    return file;
}
int main() {
    ifstream infile;
    ofstream outfile;
    ofstream outfile_printer0;
    ofstream outfile_printer1;
    ofstream outfile_harddrive;
    infile.open("definition.txt"); //the code assumes that the definition file will be named as definition.txt always and it will be placed to the same place with source code.
    outfile.open("output.txt"); //output file is named as output.txt and it is placed to the same place with source code.
    outfile_printer0.open("output_10.txt");
    outfile_printer1.open("output_11.txt");
    outfile_harddrive.open("output_12.txt");
    string a,b;
    long c;
    if (infile.is_open()){
        int iterator=1;
        while(infile >> a >> b >> c){
            Process x;
            x.id=iterator;
            x.name = a;
            x.code_file=b;
            x.arrival_time=c;
            x.last_executed_line=0;
            iterator++;
            Processes.push(x);
            howmanyprocesses++;
        }
    } //from definition.txt file, we are reading the process details and creating the Process instances.
    else{cout<<"error while opening the file, make sure that you are in the right directory"<<endl;}//if smt off occurs with definition.txt file, this error will be printed.

// assuming the first process(P1,first in the definition.txt) will arrive earliest
    TIME = Processes.front().arrival_time;
    Process front=Processes.front();
    Ready_Queue.push(front);
    update_outputfile(&outfile);
    Processes.pop();
//before going into while loop, we are initializing the ready queue by putting the first process into it. We are popping it from Processes queue since it will be either in the ready queue or completed.
    while(completed!=howmanyprocesses){
//this while loop also works when ready queue is empty.
        string instruction_name;
        bool aa=false,bb=false,cc=false;
        long duration=0;
        flag1=0;
        long duration_temp=0;
        int line_offset=0;
        string codefile_directory="";
        Process current_process;
        if(Ready_Queue.empty()){
//if ready queue is empty, we are jumping to next arrival.
            TIME = Processes.front().arrival_time;
            Process front= Processes.front();
            Ready_Queue.push(front);
            Processes.pop();
            current_process = Ready_Queue.front();
            codefile_directory=current_process.code_file;
            Ready_Queue.pop();

        }
//else we are going to current process's codefile and we pop the process from ready queue. 
//If it will be completed, we are done with it we wont put it back. But if it is not completed, we will put it to ready queue again.
        else{
            current_process = Ready_Queue.front();
            codefile_directory=current_process.code_file;
            Ready_Queue.pop();
        }
        fstream codefile;
        codefile.open(codefile_directory,fstream::in);
        GotoLine(codefile, static_cast<unsigned int>((current_process.last_executed_line) + 1));
//if quantum is not exceeded we go into while loop, since the instructions are atomic we dont halt the process if quantum is exceeded. If the previous instruction is exit, we are not go into while loop.
        char myArray[11];
        string printer="disp";
        string harddrive="read";
        char printer_array[printer.length()+1];
        strcpy(printer_array,printer.c_str());
        char harddrive_array[harddrive.length()+1];
        strcpy(harddrive_array,harddrive.c_str());
        while(duration_temp<QUANTUM && instruction_name!="exit"){
            flag=0;
            codefile>>instruction_name>>duration;
            strcpy(myArray,instruction_name.c_str());
            if(strstr(myArray,printer_array)){
                unsigned long last= instruction_name.length()-1;
                char which_printer_char = instruction_name[last];
                int which_printer_int = which_printer_char - '0';
                if(which_printer_int==0){
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    if(Printer_0_Wait_Queue.empty()){TIME_Printer0=TIME+duration_temp;}
                    Printer_0_Wait_Queue.push(current_process);
                    Printer_0_Wait_Queue_Time.push(duration);
                    update_outputfile_printer0(&outfile_printer0,TIME+duration_temp);
                    duration=0;
                    flag=1;
                    flag1=1;



                }
                else if(which_printer_int==1){
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    if(Printer_1_Wait_Queue.empty()){TIME_Printer1=TIME+duration_temp;}
                    Printer_1_Wait_Queue.push(current_process);
                    Printer_1_Wait_Queue_Time.push(duration);
                    update_outputfile_printer1(&outfile_printer1,TIME+duration_temp);
                    duration=0;
                    flag=1;
                    flag1=1;


                }
                else{
                    break;
                }

            }
            else if(strstr(myArray,harddrive_array)){
                string s = instruction_name;
                string delimiter = "_";
                string token = s.substr(0,s.find(delimiter));
                s.erase(0, s.find(delimiter) + delimiter.length());
                int which_block = atoi(s.c_str());
                if(cache_find(which_block)<0){
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    if(Hard_Drive_Wait_Queue.empty()){
                        TIME_Harddrive=TIME+duration_temp;
                    Hard_Drive_Wait_Queue.push(current_process);
                    Hard_Drive_Wait_Queue_Time.push(duration);
                    update_outputfile_harddrive(&outfile_harddrive,TIME+duration_temp);
                    cache_put(which_block);
                    duration=0;
                    flag=1;
                    flag1=1;

                }
                else{
                         duration=0; }

            }
            aa  = (TIME_Printer0+Printer_0_Wait_Queue_Time.front() <=TIME+duration_temp+duration && !Printer_0_Wait_Queue.empty());
            bb  = (TIME_Printer1+Printer_1_Wait_Queue_Time.front() <= TIME+duration_temp+duration && !Printer_1_Wait_Queue.empty());
            cc  = (TIME_Harddrive+Hard_Drive_Wait_Queue_Time.front() <= TIME+duration_temp+duration && !Hard_Drive_Wait_Queue.empty());
            if(aa){
                Process x=Printer_0_Wait_Queue.front();
                long time=Printer_0_Wait_Queue_Time.front();
                Ready_Queue.push(x);
                Printer_0_Wait_Queue.pop();
                Printer_0_Wait_Queue_Time.pop();
                TIME_Printer0=TIME+duration_temp+duration;
                update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

            }
            if(bb){
                Process x=Printer_1_Wait_Queue.front();
                long time=Printer_1_Wait_Queue_Time.front();

                Ready_Queue.push(x);
                Printer_1_Wait_Queue.pop();
                Printer_1_Wait_Queue_Time.pop();
                TIME_Printer1=TIME+duration_temp+duration;
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

            }
            if(cc){
                Process x=Hard_Drive_Wait_Queue.front();
                long time=Hard_Drive_Wait_Queue_Time.front();

                Ready_Queue.push(x);
                Hard_Drive_Wait_Queue.pop();
                Hard_Drive_Wait_Queue_Time.pop();
                TIME_Harddrive=TIME+duration_temp+duration;
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);
            }
            if(flag==1){
                break;
            }
            line_offset++;
            duration_temp+=duration;
        }
        if(flag1==0){
        current_process.last_executed_line += line_offset;}//we should record where we left.
        TIME+=duration_temp;//TIME is updated.

        if(!Processes.empty() && TIME>=Processes.front().arrival_time){
//if there are processes not arrived and their arrival time is past while we are processing the previous process, we should put it into ready queue.
            Process front=Processes.front();
            Ready_Queue.push(front);
            Processes.pop();
        }
//if the previous instruction is not exit, the process is not completed.we should put it into ready queue and update the output file.
        if(instruction_name!="exit"){
            if(flag1==0){
            Ready_Queue.push(current_process);
            }
            update_outputfile(&outfile);
        }
//if it is exit, then it is completed we are done with that process, we will increment completed by 1  and update the output file.
        else{update_outputfile(&outfile);
        completed++;}
    }
//until all processes are completed, we are continueing to do the same thing. if they are all completed, we're done.
    infile.close();
    outfile.close();
    return 0;
}

