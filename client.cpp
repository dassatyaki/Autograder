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
using namespace std;

int main(int argc,char **argv)
{
    int client_socket;

	 // Creation of socket
    client_socket = socket(AF_INET,SOCK_STREAM,0);

    if(client_socket == -1)
    {
        cout << "There was an error in creating the socket\n";
        return -1;
    }

    struct hostent *server;
    server = gethostbyname(argv[1]);

    // Inputting port number through command line
    int port = atoi(argv[2]);

    // Give address for socket
    struct sockaddr_in serv_address;
    // Erasing the data
    bzero((char *)&serv_address,sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(port);
    serv_address.sin_addr.s_addr = INADDR_ANY;

    int connection_flag;
    connection_flag = connect(client_socket,(struct sockaddr*)&serv_address,sizeof(serv_address));

    // If there was an error in connecting
    if(connection_flag == -1)
    {
        cout << "There was an error in connecting to the socket\n";
        return -1;
    }
    else 
    {
        cout << "Connected to server " << endl;
    }

	// This loop will run until user inputs "QUIT"
	while(true)
	{
		char input[1024],output[1024];
		memset(output,'\0',sizeof(output));

		cout << "Enter the command " << endl;

		string command = "";

		scanf("%[^\n]%*c",input);

		int i;
		for(i=0;i<strlen(input);++i)
		{
			if(input[i] == ' ')
				break;
			command+=input[i];
		}

		char file_name[1024],program_name[1024];
		string extn= "";

		if(command == "DELE" || command == "STOR" || command == "RETR") // To extract the file name
        {
            int j = 0;
            for(;i<strlen(input);++i)
            {
              	if(input[i] == ' ')
                    continue;
                file_name[j++] = input[i];
            }
            file_name[j] = '\0';
            if(j==0)
            {
            	cout << "No filename given for command " << command << "\n" << endl;
            	continue;
            }
        }
        else if(command == "CODEJUD") // To extract file name and extension
        {
        	int j=0;
        	
        	while(input[i] == ' ')
        	{
        		++i;
        	}

        	for(;i<strlen(input);++i)
        	{
        		if(input[i]==' ')
        			break;
        		program_name[j++] = input[i];
        	}
        	program_name[j++] = '.';

        	for(;i<strlen(input);++i)
        	{
        		if(input[i]==' ')
        			continue;
        		extn+=input[i];
        		program_name[j++] = input[i];
        	}
        	program_name[j] = '\0';

        	if(extn!="c" && extn!="cpp")
        	{
        		cout << "Invalid extension \n" << endl;
        		continue;
        	}
        }

        int read_flag,send_flag;

		if(command == "QUIT")  // Command to close the connection
		{
			send_flag =	send(client_socket,input,sizeof(input),0);
			if(send_flag == -1)
			{
				cout << "There was an error in sending\n";
			}
			else 
			{
            	cout << "Disconnected from server" << endl;
				break;
			}
		}
		else if(command == "DELE")  // Command to delete file in current directory of server
		{
			send_flag = send(client_socket,input,sizeof(input),0);
			if(send_flag == -1)
			{
				cout << "There was an error in sending\n";
			}
			else 
			{
				read_flag = recv(client_socket,output,sizeof(output),0);
				if(read_flag == -1)
				{
					cout << "There was an error in reading\n";
				}
				else 
				cout << output << endl;
			}
			bzero(output,sizeof(output));
		}
		else if(command == "LIST")  // Command to display list of files in server's directory
		{
			send_flag = send(client_socket,input,sizeof(input),0);
			if(send_flag == -1)
			{
				cout << "There was an error in sending\n";
			}
			else 
			{
				read_flag = recv(client_socket,output,sizeof(output),0);
				if(read_flag == -1)
				{
					cout << "There was an error in reading\n";
				}
				else if(strcmp(output,"The list is empty") == 0)
					cout << output << "\n" << endl;
				else 
					cout << "The list of files is \n" << output << endl;
			}
			bzero(output,sizeof(output));
		}
		else if(command == "RETR")  // Command to retrieve file from server
		{
			bool flag = false;
			if(ifstream(file_name))
			{
				cout << "The file already exists in the directory\n" << endl;
			}
			else 
			{
				send_flag = send(client_socket,input,sizeof(input),0);
				if(send_flag == -1)
				{
					cout << "There was an error in sending\n";
				}
				else 
				{
					char transfer[1024];
					
					read_flag = recv(client_socket,transfer,sizeof(transfer),0);

					if(read_flag == -1)
					{
						cout << "There was an error in reading\n";
					}
					else
					{	
						while(true)
						{
							int d = recv(client_socket,transfer,1024,0);
							if(strcmp(transfer,"END OF FILE REACHED") == 0)
								break;
							else if(strcmp(transfer,"The file you are looking for does not exist") == 0)
							{
								cout << transfer << "\n" << endl;
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
						cout << "Received file " << file_name << " from server\n" << endl;
					}
				}
			}
		}
		else if(command == "STOR")  // Command to send file to server
		{
			FILE* f ;

            send_flag = send(client_socket,input,sizeof(input),0);
			if(send_flag == -1)
			{
				cout << "There was an error in sending\n";
			}
			else
			{
				read_flag = recv(client_socket,output,sizeof(output),0);
				if(read_flag == -1)
				{
					cout << "There was an error in reading\n";
				}
				else if(strcmp(output,"The file already exists in the directory") == 0)
				{
					cout << "The file already exists at the server side \n" << endl; 
				}
				else 
				{
					if(f = fopen(file_name,"r"))
                    {
                        char transfer[1024];

                        while(fgets(transfer,1024,f)!=NULL)
                        {
                        	send(client_socket,transfer,sizeof(transfer),0);
                            if(send_flag == -1)
							{
								cout << "There was an error in sending\n";
							}
                        	bzero(transfer,1024);
                        }
                        fclose(f);
                        char message[100] = "END OF FILE REACHED";
                        send(client_socket,message,sizeof(message),0);
                        cout << file_name << " has been successfully sent to server \n" << endl;
                    }
                    else 
                    {
                        char message[256] = "The file you want to store does not exist";
			        	send(client_socket,message,strlen(message),0);
			        	if(send_flag == -1)
						{
							cout << "There was an error in sending\n";
						}
			            cout << message << "\n" << endl;         
                    }
			    }
		    }
		    bzero(output,sizeof(output));
		}
		else if(command == "CODEJUD")
		{
			FILE* f ;
			long len;

			char compile_message[256],run_message[256],verdict[256];

			// Fill blocks of memory with '\0'
			memset(compile_message,'\0',sizeof(compile_message));
			memset(run_message,'\0',sizeof(run_message));
			memset(verdict,'\0',sizeof(verdict));

            send_flag = send(client_socket,input,sizeof(input),0);
			if(send_flag == -1)
			{
				cout << "There was an error in sending\n";
			}
			else
			{
				if(f = fopen(program_name,"r"))
		        {
		            fseek(f,0,SEEK_END);
		            // Checking length of file
		            len = ftell(f);
		            // Allocating memory for character array
		            char* transfer = (char*)malloc(len+1);
		            fseek(f,0,SEEK_SET);

		            fread(transfer,len,1,f);
		            transfer[len] = '\0';
		            fclose(f);

		            send(client_socket,transfer,strlen(transfer),0);
		            if(send_flag == -1)
					{
						cout << "There was an error in sending\n";
					}
		            bzero(transfer,sizeof(transfer));

		            recv(client_socket,compile_message,sizeof(compile_message),0);
		            cout << "\n" << compile_message << "\n" << endl;

		            if(strcmp(compile_message,"COMPILE_SUCCESS") == 0)
		            {
		            	int cnt = 0;
		            	while(true)
		            	{
				            recv(client_socket,run_message,sizeof(run_message),0);

				            if(strcmp(run_message,"END_TEST") == 0)
				            	break;

				   			if(strcmp(run_message,"Input file is not present at server side") == 0)
				   			{
				   				cout << run_message << "\n" << endl;
				   				break;
				   			}

				            cout << "Testcase " << ++cnt << ":" << endl;
				            cout << run_message << "\n" << endl;

				            if(strcmp(run_message,"RUN_SUCCESS") == 0)
				            {
				            	recv(client_socket,verdict,sizeof(verdict),0);
				            	cout << verdict << "\n" << endl;
				            }
				            bzero(run_message,sizeof(run_message));
		            		bzero(verdict,sizeof(verdict));
				        }
			        }
			        // deleting the data
		            bzero(compile_message,sizeof(compile_message));
		            free(transfer);
		        }
		        else 
		        {
		        	char message[256] = "The program you want to run does not exist";
		        	send(client_socket,message,strlen(message),0);
		        	if(send_flag == -1)
					{
						cout << "There was an error in sending\n";
					}
		            cout << message << "\n" << endl;         
		        }
		    }
		    bzero(output,sizeof(output));
		}
		else if(command == "")
		{
			cout << "No command entered \n" << endl;
		}
		else
		{
			cout << "The command you have entered is invalid \n" << endl;
		}
	}	

    // Closing the client
	close(client_socket);

	return 0;
}