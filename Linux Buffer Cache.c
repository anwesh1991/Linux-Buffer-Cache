#include<stdio.h>
#include<malloc.h>
int track=0;
int i[4]={0,0,0,0};
int pro2[4];

struct buffer
{
	int devicenum;
	int blocknum;
	int status;
	int pstate;
	int new;
	struct buffer *nexthq;
	struct buffer *prevhq;
	struct buffer *nextfl;
	struct buffer *prevfl;
}*b,*flheader,*hq[4],*temp,*temphq[4],*temp2hq[4],*temp3,*temp4;

struct processsleep
{
	int sleep;
	int state;
	int held;
}p[4];

struct processstore
{
	int state[50];
}pro[4];

void makesleep(int x, int y, int z)
{
	p[x].sleep=1;
	p[x].state=y;
	p[x].held=z;
}

void wakeup(int x)
{
	p[x].sleep=0;
}

void hashtable()
{
	int check;
	check=(b->devicenum)%4;
	b->blocknum=check;
	if(i[check]==0)
	{
		hq[check]=b;
		temphq[check]=b;
		b->nexthq=NULL;
		b->prevhq=NULL;
	}
	else
	{
		b->prevhq=temphq[check];
		b->nexthq=hq[check];
		temphq[check]->nexthq=b;
		temphq[check]=b;
		hq[check]->prevhq=b;
	}
	i[check]++;
}

void displayhashtable()
{
	int j;
	printf("\nCurrently the hashtable is\n");
	printf("\nBlock number\tDevice numbers\n");
	for(j=0;j<=3;j++)
	{
		printf("%d\t\t",j);
		if(i[j]!=0)
		{
			temp2hq[j]=hq[j];
			printf("%d",temp2hq[j]->devicenum);
			temp2hq[j]=temp2hq[j]->nexthq;
			while(temp2hq[j]!=hq[j])
			{
				printf(" %d",temp2hq[j]->devicenum);
				temp2hq[j]=temp2hq[j]->nexthq;
			}
		}
		printf("\n");
	}
}

void displayfreelist()
{
	if(flheader==NULL)
	{
		printf("\nHeader<--->NULL\n");
		return;
	}
	temp3=flheader;
	printf("\nHeader<--->%d<--->",temp3->devicenum);
	temp3=temp3->nextfl;
	while(temp3!=flheader)
	{
		printf("%d<--->",temp3->devicenum);
		temp3=temp3->nextfl;
	}
	printf("Header\n");
}

void allocate(int choice)
{
	int choice2;
	int newdevicenum,check,x=0;
	printf("\nEnter the device number you want to allocate\n");
	scanf("%d",&newdevicenum);
	check=newdevicenum%4;
	temp3=hq[check];
	do
	{
		x++;
		if(temp3->devicenum==newdevicenum && temp3->status==0) //SCENARIO 1
		{
			printf("\nElement found at position %d in block number %d of hashtable. Allocation done. Process %d has now locked the buffer %d\n",x,check,choice,temp3->devicenum);
			temp3->status=1;
			temp3->pstate=choice;
			pro[choice].state[pro2[choice]]=temp3->devicenum;
			pro2[choice]++;
			if(temp3==flheader && temp3->nextfl!=flheader)
			{
				flheader=temp3->nextfl;
				flheader->prevfl=temp3->prevfl;
				(temp3->prevfl)->nextfl=flheader;
			}
			else if(temp3==flheader && temp3->nextfl==flheader)
			{
				flheader=NULL;
			}
			else
			{
				(temp3->prevfl)->nextfl=temp3->nextfl;
				(temp3->nextfl)->prevfl=temp3->prevfl;
			}
			return;
		}

		else if(temp3->devicenum==newdevicenum && temp3->status==2)
		{
			printf("Buffer has been marked for delayed write. Would you like to write its contents");
			printf("and then allocate buffer to Process %d?(1=Yes 2/Others=No)\n",choice);
			scanf("%d",&choice2);
			if(choice2==1)
			{
				printf("\nPerforming delayed write\n");
				temp3->status=1;
				temp3->pstate=choice;
				pro[choice].state[pro2[choice]]=temp3->devicenum;
				pro2[choice]++;
				if(temp3==flheader)
				{
					flheader=temp3->nextfl;
					flheader->prevfl=temp3->prevfl;
					(temp3->prevfl)->nextfl=flheader;
				}
				else
				{
					(temp3->prevfl)->nextfl=temp3->nextfl;
					(temp3->nextfl)->prevfl=temp3->prevfl;
				}
				printf("\nDelayed write done.");
				printf("Buffer %d has now been allocated to %d\n",newdevicenum,choice);
				return;
			}
			else
			{
				printf("\nNo changes made. Buffer %d is still free\n",newdevicenum);
				return;
			}
		}

		else if(temp3->devicenum==newdevicenum && temp3->status==1 && temp3->pstate==choice)
		{
			printf("\nBuffer %d is already being used by Process",temp3->devicenum);
			printf(" %d.No changes done\n",choice);
			return;
		}

		else if(temp3->devicenum==newdevicenum && temp3->status==1)
		{
			printf("\nElement found at position %d in block number %d of the hashtable",x,check);
			printf("but it is currently busy as it is in use by Process %d.",temp3->pstate);
			printf("Use Process %d's deallocation function if you want to free it\n",temp3->pstate);
			makesleep(choice,temp3->devicenum,temp3->pstate);
			printf("\nProcess %d is now sleeping and waiting for",choice);
			printf(" buffer %d to be freed by Process %d",temp3->devicenum,temp3->pstate);
			return;temp3->status==2;
		}

		else
			temp3=temp3->nexthq;

	}while(temp3!=hq[check]);

	if(temp3==hq[check])
	{
		if(flheader==NULL) //SCENARIO 4
		{
			printf("\nFreelist is empty. Putting process %d to sleep\n",choice);
			makesleep(choice,newdevicenum,4);
			return;
		}
		printf("Element not found in hash queue.");
		printf("Going to remove one element from the freelist to allocate new buffer\n");
		temp4=flheader;
		do
		{
			if(temp4->status==2)  //SCENARIO 3
			{
				printf("\nDelayed Write buffer %d found. Writing to disk\n",temp4->devicenum);
				temp4->status=0;
				temp4->nextfl=flheader;
				temp4->prevfl=flheader->prevfl;
				(flheader->prevfl)->nextfl=temp4;
				flheader->prevfl=temp4;
				flheader=temp4;
				printf("/n %d buffer written to disk and added",temp4->devicenum);
				printf(" to the front of the freelist/n");
			}
			else //SCENARIO 2
			{
				printf("\nbuffer %d removed from freelist\n",temp4->devicenum);
				printf("\nAdding new element to correct hash queue\n");
				b=(struct buffer*)malloc(sizeof(struct buffer));
				b->devicenum=newdevicenum;
				(flheader->prevfl)->nextfl=flheader->nextfl;
				(flheader->nextfl)->prevfl=flheader->prevfl;
				(flheader->nexthq)->prevhq=flheader->prevhq;
				(flheader->prevhq)->nexthq=flheader->nexthq;
				if(flheader==hq[(flheader->devicenum%4)])
				hq[(flheader->devicenum%4)]=hq[(flheader->devicenum%4)]->nexthq;
				flheader=flheader->nextfl;
				(hq[check]->prevhq)->nexthq=b;
				b->prevhq=hq[check]->prevhq;
				hq[check]->prevhq=b;
				b->nexthq=hq[check];
				temp3=temp3->prevhq;
				temp3->status=1;
				temp3->pstate=choice;
				pro[choice].state[pro2[choice]]=temp3->devicenum;
				pro2[choice]++;
				printf("\nElement has been added to the hash queue and buffer");
				printf("%d has been allocated to process %d\n",temp3->devicenum,choice);
				return;
			}
		}while(temp4=flheader);
	}

}

void deallocate(int choice)
{
	int z;
	int choice2,buff,check,choice3;
	printf("\n Process %d is currently working on these processes(list order : old to recent)\n",choice);
	for(z=0;z<pro2[choice];z++)
	printf("\n%d",pro[choice].state[z]);
	label234:
	printf("\nHow do you want to deallocate?\n1.Deallocate a particular buffer\n2.Deallocate all buffers\n3.Exit\n");
	scanf("%d",&choice2);
	while(1)
	{
		switch(choice2)
		{
			label123:
			case 1:printf("\nWhich buffer would you like to deallocate?\n"); //Switch case 1
			scanf("%d",&buff);
			for(z=0;z<pro2[choice];z++)
			{
				if(pro[choice].state[z]==buff)
				break;
			}
			if(z==pro2[choice]+1)
			{
				printf("Element not found. Retry");
				goto label123;
			}
			else
			{
				check=buff%4;
				temp3=hq[check];
				do
				{

					if(temp3->devicenum==buff && temp3->status==1)
					{
						printf("\nGoing to deallocate buffer %d.Would you like to mark it for delay write?(1=Yes, 2/Others=No)\n",temp3->devicenum);
						scanf("%d",&choice3);
						if(choice3==1)
							temp3->status=2;
						else
							temp3->status=0;
						(flheader->prevfl)->nextfl=temp3;
						temp3->prevfl=flheader->prevfl;
						temp3->nextfl=flheader;
						flheader->prevfl=temp3;
						printf("\nDeallocation complete. Node placed at the end of freelist and isavailable for use by other processes\n");
						if(p[1].sleep==1 && p[1].state==buff)
						{
						    wakeup(1);
						    printf("\nAs %d was being held by %d, and Process 1 went into sleep mode as a result, Process 1 has now been woken up\n",buff,choice);
						}
						else if(p[2].sleep==1 && p[2].state==buff)
						{
						    wakeup(2);
						    printf("\nAs %d was being held by %d, and Process 2 went into sleep mode as a result, Process 2 has now been woken up\n",buff,choice);
						}

						else if(p[3].sleep==1 && p[3].state==buff)
						{
						    wakeup(3);
						    printf("\nAs %d was being held by %d, and Process 3 went into sleep mode as a result, Process 3 has now been woken up\n",buff,choice);
						}
						else
						return;
						pro2[choice]--;

					}
					else
					{
						temp3=temp3->nexthq;
					}
				}while(temp3!=hq[check]);
			}
		return;


			case 2:
			for(z=0;z<pro2[choice];)
			{
				check=pro[choice].state[z]%4;
				temp3=hq[check];
				printf("\n%d\n",pro[choice].state[z]);
				do
				{
					if(temp3->status==1)
					{
						printf("\nGoing to deallocate the buffer %d. Would you like to mark it for delayed write?(1=Yes, 2/Others=No)\n",temp3->devicenum);
						scanf("%d",&choice3);
						if(choice3==1)
							temp3->status=2;
						else
							temp3->status=0;
						if(flheader==NULL)
						{
							flheader=temp3;
						}
						else
						{
							(flheader->prevfl)->nextfl=temp3;
							temp3->prevfl=flheader->prevfl;
							temp3->nextfl=flheader;
							flheader->prevfl=temp3;
							printf("\nDeallocation complete. Node placed at the end of freelist and is available for use by other processes\n");
							if(p[1].sleep==1 && p[1].state==temp3->devicenum)
							{
							    wakeup(1);
							    printf("\nAs %d was being held by %d, and Process 1 went into sleep mode as a result, Process 1 has now been woken up\n",temp3->devicenum,choice);
							}
							else if(p[2].sleep==1 && p[2].state==temp3->devicenum)
							{
							    wakeup(2);
							    printf("\nAs %d was being held by %d, and Process 2 went into sleep mode as a result, Process 2 has now been woken up\n",temp3->devicenum,choice);
							}
							else if(p[3].sleep==1 && p[3].state==temp3->devicenum)
							{
							    wakeup(3);
							    printf("\nAs %d was being held by %d, and Process 3 went into sleep mode as a result, Process 3 has now been woken up\n",temp3->devicenum,choice);
							}
							else
							{}
						}
					goto labelxyz;
					}
					else
					{
						temp3=temp3->nexthq;
					}
				}while(temp3!=hq[check]);
			labelxyz:	
			z++;
			}
		pro2[choice]=0;
		return;
		case 3:
		return;

		default:
		printf("\nInvalid entry. Please re-enter\n");
		goto label234;
		}
	}
}


int main()
{
	int device[]={3,50,97,4,64,5,202,99,35,98,17,28,40,62,53,103,0},choice,i=0,choice2;
	char c;
	printf("\nWelcome to Linux buffer cache mechanism simulation\n");
	b=(struct buffer*)malloc(sizeof(struct buffer));
	b->devicenum=device[0];
	flheader=b;
	temp=b;
	b->nextfl=NULL;
	b->prevfl=NULL;
	b->status=0;
	track++;
	hashtable();
	i++;
	while(device[i]!=0)
	{
		track++;
		b=(struct buffer*)malloc(sizeof(struct buffer));
		b->devicenum=device[i];
		temp->nextfl=b;
		b->nextfl=flheader;
		b->prevfl=temp;
		b->status=0;
		temp=b;
		flheader->prevfl=b;
		hashtable();
		i++;
	}
	displayhashtable();
	printf("\n\n");
	label1:
	while(1)
	{
		printf("\nWhat would you like to do?\n\n1.Use Process 1\n2.Use Process 2\n3.Use Process 3\n4.Display Hashtable\n5.Display Freelist\n6.Exit\n");
		scanf("%d",&choice);
		switch(choice)
		{

			case 1:
			{
				
				
					printf("\nWhat do you want to do?\n\n1.Allocate a buffer\n2.Deallocate a busy buffer\n");
					scanf("%d",&choice2);
					switch(choice2)
					{
						case 1:
						if(p[1].sleep==1 && p[1].held!=4)
						{
							printf("Process 1 is currently asleep and the allocation function cannot be used until %d is freed by %d",p[1].state,p[1].held);
							break;
						}
						else if(p[1].sleep==1 && p[1].held==4)
						{
							printf("Process 1 is currently asleep as the freelist is empty and the allocation function cannot be used Process 1 will be assigned to new buffer %d after any deallocation has taken place",p[1].state);
							break;
						}
						else
						{
							allocate(1);
							break;
						}
						case 2:deallocate(1);
						break;
						default:
						printf("\nInvalid entry. Enter again\n");
					}
					break;
				
			}

			case 2:
			{
				
				
					printf("\nWhat do you want to do?\n\n1.Allocate a buffer\n2.Deallocate a busy buffer\n");
					scanf("%d",&choice2);
					switch(choice2)
					{
						case 1:
						if(p[2].sleep==1 && p[2].held!=4)
						{
							printf("Process 2 is currently asleep and allocation function cannot be used until %d is freed by %d",p[1].state,p[1].held);
							break;
						}
						else if(p[2].sleep==1 && p[2].held==4)
						{
							printf("Process 2 is currently asleep as the freelist is empty and the allocation function cannot be used. Process 2 will be assigned to new buffer %d after any deallocation has taken place",p[1].state);
							break;
						}
						else
						{
							allocate(2);
							break;
						}
						case 2:deallocate(2);
						break;
						default:
						printf("\nInvalid entry. Enter again\n");
					}
					break;
				
			}
			case 3:
			{
				
				
					printf("\nWhat do you want to do?\n\n1.Allocate a buffer\n2.Deallocate a busy buffer\n");
					scanf("%d",&choice2);
					switch(choice2)
					{
						case 1:
						if(p[3].sleep==1 && p[3].held!=4)
						{
							printf("Process 2 is currently asleep and allocation function cannot be used until %d is freed by %d",p[1].state,p[1].held);
							break;
						}
						else if(p[3].sleep==1 && p[3].held==4)
						{
							printf("Process 2 is currently asleep and allocation function cannot be used as the freelist is empty. Process 2 will be assigned to new buffer %d after any deallocation has taken place",p[1].state);
							break;
						}
						else
						{
							allocate(3);
							break;
						}
						case 2:deallocate(3);
						break;
						default:
						printf("\nInvalid entry. Enter again\n");
					}
					break;
				
			}

			case 4:displayhashtable();
				break;

			case 5:displayfreelist();
				break;

			case 6:
				goto label2;

			default:
				printf("\nInvalid entry. Enter again\n");
				break;
		}
	}

	label2:
	return 0;
}
