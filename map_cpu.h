//mapping table data structure is used to store the correspondence between threads and core.	author：hh
typedef struct processor{
	int logical_id;
	int physical_id;
}Proc;

typedef struct index{
	int proc_no;
	int thread_no;
}Index;
 
////mapping function: returns the cpu number bound to the thread in same core	author：hh
int mapping(int proc_num,int thread_num,int t_no){
	Index* index=(Index*)malloc(thread_num*sizeof(Index));
	int tnum=0,cnum=0;
	while(tnum<thread_num){
		index[tnum].thread_no=tnum;
		index[tnum].proc_no=cnum;
		tnum++;
		cnum++;
		if(cnum>=proc_num){
			cnum=0;		
		}
	} 
	return index[t_no].proc_no;      
}

//mapping function in different core
int mapping_diff_core(int proc_num,int thread_num,int t_no){
	Index* index=(Index*)malloc(thread_num*sizeof(Index));
	int tnum=0,cnum=0;
	while(tnum<thread_num){
		index[tnum].thread_no=tnum;
		index[tnum].proc_no=cnum;
		tnum++;
		cnum=cnum+2;
		if(cnum>=proc_num){
			cnum=1;	
		}
	} 
	return index[t_no].proc_no;      
}
//read config file
/*char* read_file(char * filename){
	FILE *fp=fopen(filename, "r+");
	char buf[1024];
	if (fp != (FILE *)NULL)
	{
		while(fgets(buf, sizeof(buf), fp)){
			printf("%s",buf);
		}
	fclose(fp);
	} 
	return buf;
}*/
