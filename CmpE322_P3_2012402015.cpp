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
int flag=0;
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
                cache[i].blocknum=value;
                cache[i].last_used=0;
                return false;
            }
        }
        cache[0].blocknum=value;
        cache[0].last_used=0;
        cache[1].last_used=1;
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
    infile.open("/home/mmervecerit/CLionProjects/cmpe322/definition.txt"); //the code assumes that the definition file will be named as definition.txt always and it will be placed to the same place with source code.
    outfile.open("/home/mmervecerit/CLionProjects/cmpe322/output2.txt"); //output file is named as output.txt and it is placed to the same place with source code.
    outfile_printer0.open("/home/mmervecerit/CLionProjects/cmpe322/output2_10.txt");
    outfile_printer1.open("/home/mmervecerit/CLionProjects/cmpe322/output2_11.txt");
    outfile_harddrive.open("/home/mmervecerit/CLionProjects/cmpe322/output2_12.txt");
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
        flag=0;
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
            cout<<"Current process is "<<current_process.name<<" and time= "<<TIME<<endl;
            codefile_directory="/home/mmervecerit/CLionProjects/cmpe322/"+current_process.code_file;
            Ready_Queue.pop();

        }
//else we are going to current process's codefile and we pop the process from ready queue. 
//If it will be completed, we are done with it we wont put it back. But if it is not completed, we will put it to ready queue again.
        else{
            current_process = Ready_Queue.front();
            cout<<"Current process is "<<current_process.name<<" and time= "<<TIME<<endl;
            codefile_directory="/home/mmervecerit/CLionProjects/cmpe322/"+current_process.code_file;
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
            codefile>>instruction_name>>duration;
            cout<< "Current Process: "<<current_process.name<<" Instr: "<<instruction_name<<" Duration: "<<duration<<" Duration_temp: "<<duration_temp<<"TIME: "<<TIME<<endl;
            strcpy(myArray,instruction_name.c_str());
            if(strstr(myArray,printer_array)){
                cout<<"Current Process wants Printer"<<endl;
                unsigned long last= instruction_name.length()-1;
                char which_printer_char = instruction_name[last];
                int which_printer_int = which_printer_char - '0';
                if(which_printer_int==0){
                    cout<<"Current Process wants Printer-0, we will put it to waiting que"<<endl;
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    Printer_0_Wait_Queue.push(current_process);
                    TIME_Printer0=TIME+duration_temp+duration;
                    cout<<"We should remove the process from Printer-0 at "<<TIME_Printer0<<endl;
                    update_outputfile_printer0(&outfile_printer0,TIME+duration_temp);
                    duration=0;
                    flag=1;
                    break;

                }
                else if(which_printer_int==1){
                    cout<<"Current Process wants Printer-1, we will put it to waiting que"<<endl;
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    Printer_1_Wait_Queue.push(current_process);
                    TIME_Printer1=TIME+duration_temp+duration;
                    cout<<"We should remove the process from Printer-1 at "<<TIME_Printer1<<endl;
                    update_outputfile_printer1(&outfile_printer1,TIME+duration_temp);
                    duration=0;
                    flag=1;
                    break;
                }
                else{
                    cout<<"ERROR READING PRINTER INFO"<<endl;
                    break;
                }

            }
            else if(strstr(myArray,harddrive_array)){
                string s = instruction_name;
                string delimiter = "_";
                string token = s.substr(0,s.find(delimiter));
                s.erase(0, s.find(delimiter) + delimiter.length());
                int which_block = atoi(s.c_str());
                cout<<"Current Process wants Harddrive with blocknumber: "<<which_block<<endl;
                if(cache_find(which_block)<0){
                    cout<<"cachete yok o block, waiting queya at"<<endl;
                    line_offset++;
                    current_process.last_executed_line+=line_offset;
                    Hard_Drive_Wait_Queue.push(current_process);
                    update_outputfile_harddrive(&outfile_harddrive,TIME+duration_temp);
                    cache_put(which_block);
                    TIME_Harddrive=TIME+duration_temp+duration;
                    cout<<"We should remove the process from Harddrive at "<<TIME_Harddrive<<endl;
                    duration=0;
                    flag=1;
                    break;
                }
                else{
                        cout<<"cachede bulduk, gerek yok waitinge, hicbisi olmamıs gibi devam et"<<endl;
                         duration=0;}
            }
            line_offset++;
            duration_temp+=duration;
        }
        if(flag==0){
        current_process.last_executed_line += line_offset;}//we should record where we left.
        cout<<"Current Processin su ve surasında kaldın: "<<current_process.name<<" - "<<current_process.last_executed_line<<endl;
        TIME+=duration_temp;//TIME is updated.
        cout<<"Time yenilendi: "<<TIME<<endl;
        aa  = (TIME_Printer0 <=TIME && !Printer_0_Wait_Queue.empty());
        bb  = (TIME_Printer1 <= TIME && !Printer_1_Wait_Queue.empty());
        cc  = (TIME_Harddrive <= TIME && !Hard_Drive_Wait_Queue.empty());
        if( aa   &&  bb   && cc){
            long mi=(min(TIME_Printer0,TIME_Printer1),TIME_Harddrive);
            if((mi==TIME_Harddrive && mi == TIME_Printer0 && mi == TIME_Printer1) || (mi!=TIME_Harddrive && mi == TIME_Printer0 && mi == TIME_Printer1)){
                Process p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

                p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);
            }
            else if(mi==TIME_Harddrive && mi==TIME_Printer0 && mi!=TIME_Printer1){
                Process p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

                p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);

                p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
            }
            else if(mi==TIME_Harddrive && mi== TIME_Printer1 && mi!=TIME_Printer0){
                Process p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);

                p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME_Printer0);
            }
            else if(mi==TIME_Printer0 && mi!=TIME_Printer1 && mi!=TIME_Harddrive){
                Process p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

                long mii = min(TIME_Printer1,TIME_Harddrive);
                if(mii!=TIME_Printer1 && mii==TIME_Harddrive){
                    Process t = Hard_Drive_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Hard_Drive_Wait_Queue.pop();
                    update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);
                    t = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
                }
                else{
                    Process t = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                    t = Hard_Drive_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Hard_Drive_Wait_Queue.pop();
                    update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);
                }
            }

            else if(mi   == TIME_Printer1   &&  mi !=   TIME_Printer0   &&  mi !=   TIME_Harddrive){
                Process p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                long mii = min(TIME_Printer0,TIME_Harddrive);
                if(mii!=TIME_Printer0 && mii==TIME_Harddrive){
                    Process p = Hard_Drive_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Hard_Drive_Wait_Queue.pop();
                    update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);

                    p = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME_Printer0);
                }
                else{
                    Process t = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

                    t = Hard_Drive_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Hard_Drive_Wait_Queue.pop();
                    update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);

                }
            }
            else if(mi   != TIME_Printer1   &&  mi !=   TIME_Printer0   &&  mi ==   TIME_Harddrive){
                Process p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);

                long mii = min(TIME_Printer0,TIME_Printer1);
                if(mii!=TIME_Printer0 && mii==TIME_Printer1){
                    Process t = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                    t = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME_Printer0);
                }
                else{
                    Process t = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME_Printer0);

                    t = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<t.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(t);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
                }
            }
        }
        else if (aa && bb && !cc){
            long mii = min(TIME_Printer0,TIME_Printer1);
                if(mii!=TIME_Printer0 && mii==TIME_Printer1){
                    Process p = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);

                    p = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME);
                }
                else{
                    Process p = Printer_0_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Printer_0_Wait_Queue.pop();
                    update_outputfile_printer0(&outfile_printer0,TIME);

                    p = Printer_1_Wait_Queue.front();
                    cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                    Ready_Queue.push(p);
                    Printer_1_Wait_Queue.pop();
                    update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
                }
        }
        else if (aa && cc && !bb){
            long mii = min(TIME_Printer0,TIME_Harddrive);
            if(mii!=TIME_Printer0 && mii==TIME_Harddrive){
                Process p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME);

                p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME);
            }
            else{
                Process p = Printer_0_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_0_Wait_Queue.pop();
                update_outputfile_printer0(&outfile_printer0,TIME);

                p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME);

            }
        }
        else if (bb && cc && !aa){
            long mii = min(TIME_Printer1,TIME_Harddrive);
            if(mii!=TIME_Printer1 && mii==TIME_Harddrive){
                Process p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME);
                p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
            }
            else{
                Process p = Printer_1_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Printer_1_Wait_Queue.pop();
                update_outputfile_printer1(&outfile_printer1,TIME_Printer1);
                p = Hard_Drive_Wait_Queue.front();
                cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
                Ready_Queue.push(p);
                Hard_Drive_Wait_Queue.pop();
                update_outputfile_harddrive(&outfile_harddrive,TIME_Harddrive);
            }
        }
        else if(aa && !bb && !cc){
            Process p = Printer_0_Wait_Queue.front();
            cout<<"Process "<<p.name<<" in  zamanı geldi printer-0 dan cıkartıp ready queya koy."<<endl;
            Ready_Queue.push(p);
            Printer_0_Wait_Queue.pop();
            update_outputfile_printer0(&outfile_printer0,TIME);
        }
        else if(bb && !aa && !cc){
            Process p = Printer_1_Wait_Queue.front();
            cout<<"Process "<<p.name<<" in  zamanı geldi printer-1 dan cıkartıp ready queya koy."<<endl;
            Ready_Queue.push(p);
            Printer_1_Wait_Queue.pop();
            update_outputfile_printer1(&outfile_printer1,TIME);
        }
        else if(cc && !aa && !bb){
            Process p = Hard_Drive_Wait_Queue.front();
            cout<<"Process "<<p.name<<" in  zamanı geldi harddrive dan cıkartıp ready queya koy."<<endl;
            Ready_Queue.push(p);
            Hard_Drive_Wait_Queue.pop();
            update_outputfile_harddrive(&outfile_harddrive,TIME);
        }

        if(!Processes.empty() && TIME>=Processes.front().arrival_time){ 
//if there are processes not arrived and their arrival time is past while we are processing the previous process, we should put it into ready queue.
            Process front=Processes.front();
            cout<<"Yeni process geldi hanıım:  "<<front.name<<endl;
            Ready_Queue.push(front);
            Processes.pop();
        }
//if the previous instruction is not exit, the process is not completed.we should put it into ready queue and update the output file.
        if(instruction_name!="exit"){
            if(flag==0){
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
