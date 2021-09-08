#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h> 
using namespace std;

// For comparison of the two strings
bool isCorrect(string a,string b)
{
	bool flag = true;
	
	if(a[a.length()-1] == '\n' || a[a.length()-1] == '\r') 
	a = a.substr(0,a.length()-1);
	if(b[b.length()-1] == '\n' || b[b.length()-1] == '\r')
	b = b.substr(0,b.length()-1);

	if(a!=b)
	{
		flag = false;
	}

	return flag;
}

int main(int argc,char **argv)
{
	// File descriptor sets 
	fd_set fds,nw_fds;

	struct sockaddr_in addr;

	int server_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,0);

    // Creation of socket
    if(server_socket == -1)
    {
        cout << "There was an error in creating the socket\n";
        return -1;
    }
    int opt = 1,set_flag;
    set_flag = setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt));

    if(set_flag == -1)
    {
        cout << "There was an error in attaching the socket to port\n";
        return -1;
    }

    // Inputting port number through command line
    int port = atoi(argv[1]);

    // Give address for socket
    struct sockaddr_in serv_address;
    // Erasing the data
    bzero((char *) &serv_address, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(port);
    serv_address.sin_addr.s_addr = INADDR_ANY;

    int bind_flag;
    bind_flag = bind(server_socket,(struct sockaddr*)&serv_address,sizeof(serv_address));
    if(bind_flag == -1)
    {
        cout << "There was an error in binding\n";
        return -1;
    }

    int listen_flag;
    // Listening for connections from clients
    listen_flag = listen(server_socket,10);
    if(listen_flag == -1)
    {
        cout << "There was an error in listening to the client\n";
        return -1;
    }

    socklen_t size_addr = sizeof(addr);	

    FD_ZERO(&fds);
    FD_SET(server_socket,&fds);

    while(true)
    {
    	char input[1024],file_name[1024],program_name[1024];
    	memset(input,'\0',sizeof(input));

        string command = "", inp = "";

    	nw_fds = fds;

    	int select_flag;
    	select_flag = select(FD_SETSIZE,&nw_fds,NULL,NULL,NULL);

    	if(select_flag == -1)
    	{
    		cout << "There was an error in select\n";
    		return -1;
    	}
    	int read_flag,send_flag;
    	string extn = "",prog = "",name = "",run = "";

    	for(int i=0;i<FD_SETSIZE;++i)
    	{
    		if(FD_ISSET(i,&nw_fds))
    		{
    			if(server_socket == i)
    			{
    				int client_socket = accept(server_socket,(struct sockaddr*)&addr,&size_addr);

    				if(client_socket == -1)
    				{
    					cout << "There was an error in client socket\n";
    					return -1;
    				}
    				else 
    				{
    					FD_SET(client_socket,&fds);
    					cout << "New connection from " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
    				}
    			}
    			else 
    			{
    				// Read data from client
	    			read_flag = read(i,input,sizeof(input));
	    			if(read_flag == -1)
	    			{
	    				cout << "There was an error in reading\n";
	    			}
	    			else 
	    			{
	                    int k;
	                    for(k=0;i<strlen(input);++k)
	                    {
	                        if(input[k] == ' ')
	                            break;
	                        command+=input[k];
	                    }

	                    if(command!="")
	                        cout << command << " Command received from client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
	                    else 
	                        cout << input << " Command received from client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;

		    			if(strcmp(input,"QUIT") == 0)
					    {
					    	cout << "Connection disconnected from " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
					    	close(i);
					    	FD_CLR(i,&fds);
					    	break;
					    }

	                    if(command == "DELE" || command == "STOR" || command == "RETR") // To extract the file name
	                    {
	                        int j = 0;
	                        for(;k<strlen(input);++k)
	                        {
	                            if(input[k] == ' ')
	                                continue;
	                            file_name[j++] = input[k];
	                        }
	                        file_name[j] = '\0';
	                    }
	                    else if(command == "CODEJUD") // To extract file name and extension
				        {
				        	int j=0;
				        	
				        	while(input[k] == ' ')
				        	{
				        		++k;
				        	}

				        	for(;k<strlen(input);++k)
				        	{
				        		if(input[k]==' ')
				        			break;
				        		program_name[j++] = input[k];
				        		name+=input[k];
				        	}
				        	program_name[j++] = '.';

				        	for(;k<strlen(input);++k)
				        	{
				        		if(input[k]==' ')
				        			continue;
				        		program_name[j++] = input[k];
				        		extn+=input[k];
				        	}
				        	program_name[j] = '\0';
				        }

				        string pr(program_name);
				        pr=to_string(ntohs(addr.sin_port))+pr;

	                    if(command == "DELE") // Command to delete file in current directory of server
	                    {
	                        FILE* f;

	                        if(f = fopen(file_name,"r"))
	                        {
	                            int delete_flag = remove(file_name);
	                            if(delete_flag == 0)
	                            {
	                                char message[256] = "File deleted successfully\n";
	                                send_flag = send(i,message,strlen(message),0);
	                                if(send_flag == -1)
									{
										cout << "There was an error in sending\n";
									}
	                            }
	                            else 
	                            {
	                                char message[256] = "File could not be deleted\n";
	                                send_flag = send(i,message,strlen(message),0);
	                                if(send_flag == -1)
									{
										cout << "There was an error in sending\n";
									}
	                            }
	                            fclose(f);
	                        }
	                        else 
	                        {
	                            char message[256] = "The file you are trying to delete does not exist\n";
	                            send_flag = send(i,message,strlen(message),0);
	                            if(send_flag == -1)
								{
									cout << "There was an error in sending\n";
								}
	                        }
	                    }
	                    else if(strcmp(input,"LIST") == 0) // Command to display list of files in server's directory
	                    {
	                        char files[1024],message[256] = "Problem in opening directory ";
	                        memset(files,'\0',sizeof(files));
	                        DIR* d;
	                        d = opendir(".");
	                        struct dirent* dr;

	                        int p = 0;
	                        if(d)
	                        {
	                            while((dr = readdir(d))!=NULL)
	                            {
	                                if(strcmp(dr->d_name,".")!=0 && strcmp(dr->d_name,"..")!=0 && strcmp(dr->d_name,"server.cpp")!=0)
	                                {
	                                    for(int k=0;k<strlen(dr->d_name);++k)
	                                    {
	                                        files[p++] = dr->d_name[k];
	                                    }
	                                    files[p++] = '\n';
	                                }
	                            }
	                            closedir(d);
	                            if(p == 0)
	                            {
	                            	char message[256] = "The list is empty";
  	                            	send_flag = send(i,message,strlen(message),0);
	                            }
	                            else 
	                            {
	                            	send_flag = send(i,files,strlen(files),0);
	                            }
	                            if(send_flag == -1)
								{
									cout << "There was an error in sending\n";
								}
								else 
									cout << "Sent list of files to client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;

	                        }
	                        else 
	                        {
	                            send_flag = send(i,message,strlen(message),0);
	                            if(send_flag == -1)
								{
									cout << "There was an error in sending\n";
								}
	                        }
	                     	bzero(files,sizeof(files));   
	                    }
	                    else if(command == "RETR") // Command to send file to client
	                    {
	                        FILE* f ;
	                        long len;

	                        if(f = fopen(file_name,"r"))
	                        {
	                            char transfer[1024];

	                            while(fgets(transfer,1024,f)!=NULL)
	                            {
	                            	send(i,transfer,sizeof(transfer),0);
		                            if(send_flag == -1)
									{
										cout << "There was an error in sending\n";
									}
	                            	bzero(transfer,1024);
	                            }
	                            fclose(f);
	                            char message[100] = "END OF FILE REACHED";
	                            send(i,message,sizeof(message),0);
	                            cout << file_name << " has been successfully sent to client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
	                        }
	                        else 
	                        {
	                            char message[1024] = "The file you are looking for does not exist";
	                  
	                            send_flag = send(i,message,sizeof(message),0);
	                            send(i,message,sizeof(message),0);
	                            if(send_flag == -1)
								{
									cout << "There was an error in sending\n";
								}
	                        }
	                    }
	                    else if(command == "STOR") // Command to receive file from client
	                    {
	                    	bool flag = false;
	                    	if(ifstream(file_name))
							{
								char message[256] = "The file already exists in the directory";
								send(i,message,strlen(message),0);
								if(send_flag == -1)
								{
									cout << "There was an error in sending\n";
								}
							}
							else 
							{
								char transfer[1024];
								memset(transfer,'\0',1024);
								char message[100] = "Send the file now";
								send(i,message,sizeof(message),0);
								while(true)
								{
									int d = recv(i,transfer,1024,0);
									if(strcmp(transfer,"END OF FILE REACHED") == 0)
										break;
									else if(strcmp(transfer,"The file you want to store does not exist") == 0)
									{
										break;
									}
									else 
									{
										fstream f;
										f.open(file_name,ios::app);
										f << transfer;
										f.close();
										flag = true;
									}
									bzero(transfer,1024);
								}
								if(flag == true)
								cout << "Received file " << file_name << " from client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
							}
	                    }
	                    else if(command == "CODEJUD")
	                    {
	                    	char transfer[99024];
								
							read_flag = recv(i,transfer,sizeof(transfer),0);

							if(read_flag == -1)
							{
								cout << "There was an error in reading\n";
							}
							else if(strcmp(transfer,"The program you want to run does not exist") == 0)
							{
								cout << transfer << endl;
							}
							else 
							{
								// To store the time of execution of program
								time_t begin,terminate;
								ofstream f;
								f.open(pr.c_str());

								cout << "Received program file " << program_name << " from client with IP address and port " << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "\n" << endl;
								f << transfer;
								f.close();

								bzero(transfer,sizeof(transfer));

								 if(extn == "c")
						        	prog = "gcc ";
						        else if(extn == "cpp")
						        	prog = "g++ ";

						        string comp_error_file = "compile_error_" + name + to_string(ntohs(addr.sin_port)) + ".txt";
						        string run_error_file = "run_error_" + name + to_string(ntohs(addr.sin_port)) + ".txt";
						        prog+= "-o " + name + to_string(ntohs(addr.sin_port)) + " " + to_string(ntohs(addr.sin_port)) + name + "." + extn + " 2> " + comp_error_file;
						        run = "timeout 1 ./" + name + to_string(ntohs(addr.sin_port));
						        inp = "input_" + name + ".txt";
								const char* comp = prog.c_str();
								string test_file = "testcase_" + name + ".txt",out_file = "output_" + name + to_string(ntohs(addr.sin_port)) + ".txt";
								int system_flag = system(comp);
								run+= " < in_" + name + to_string(ntohs(addr.sin_port)) + ".txt > output_" + name + to_string(ntohs(addr.sin_port)) + ".txt 2> " + run_error_file;
								remove(comp_error_file.c_str());

								if(system_flag == 0)
								{
									char message[256] = "COMPILE_SUCCESS",run_message[256];
									send_flag = send(i,message,sizeof(message),0);
									if(send_flag == -1)
									{
										cout << "There was an error in sending\n";
									}
									else
									{
										if(ifstream(inp))
										{
											fstream fi,f1;
											string dummy_input;
											const char* input_file = inp.c_str();
											string str,test;
											fi.open(input_file,ios::in);
											f1.open(test_file,ios::in);

											while(getline(fi,str) && getline(f1,test))
											{
												ofstream fp;
												dummy_input = "in_" + name + to_string(ntohs(addr.sin_port)) + ".txt";
												fp.open(dummy_input.c_str());
												fp << str << endl;
												const char* runner = run.c_str();
												// Opening time
	    										time(&begin);
												int system_flag_run = system(runner);
												// Closing time
										    	time(&terminate);
							
												double t = double(terminate)-double(begin);

												ifstream fr(run_error_file,ios::ate);

												if(fr.tellg() == 0)
													strcpy(run_message,"RUN_SUCCESS");
												else 
													strcpy(run_message,"RUN_ERROR");

												// If the time required is more than 1 second
												if(t>=1)
													strcpy(run_message,"TIME LIMIT EXCEEDED");

												send(i,run_message,sizeof(run_message),0);
												remove(run_error_file.c_str());
												
												if(strcmp(run_message,"RUN_SUCCESS") == 0)
												{
													ifstream f2(out_file);

													string out;

													getline(f2,out);

													bool flag = isCorrect(test,out);
													string verdict;
													
													if(flag)
														verdict = "ACCEPTED";
													else 
														verdict = "WRONG_ANS";

													const char* ver = verdict.c_str();
													send(i,ver,strlen(ver),0);
												}
												remove(dummy_input.c_str());
												fp.close();
												// Deleting the output file once done
												remove(out_file.c_str()); 
											}
											char end[256] = "END_TEST";
											send(i,end,sizeof(end),0);
											fi.close();
										}
										else 
										{
											char message1[256] = "Input file is not present at server side";
											send_flag = send(i,message1,strlen(message1),0);
											if(send_flag == -1)
											{
												cout << "There was an error in sending\n";
											}
											cout << message1 << "\n";
										}
									}
								}
								else if(system_flag!=0)
								{
									char message2[256] = "COMPILE_ERROR";
									send_flag = send(i,message2,sizeof(message2),0);
									if(send_flag == -1)
									{
										cout << "There was an error in sending\n";
									}
								}
							}
	                    }
                    }
                }
            }
        }
    }

    return 0;
}