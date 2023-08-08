#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include "cJSON.h"
#include<conio.h>
#define ROW 5
#define PRINT Init

//全局变量存储账户json文件对象
cJSON*acc_root=NULL;
cJSON*words_root=NULL;
//定义words结构体存储单词以及其释义
typedef struct words
{
    char words[16];
    char Chinese_meaning[36];
    char English_meaning[128];
}WORDS;

//定义accounts结构体存储账户信息
typedef struct accounts
{
    char name[24];
    char password[24];
    int count;//记录次数
}ACC;

void Init(int COL,char arr[][COL+1],int count[][COL]);
void RandomWords(WORDS *words,char filename[]);
void Game(WORDS* words,int COL,char arr[][COL+1],ACC *acc);
int  Words_cmp(WORDS* words,int COL,char arr[],int count[]);
void Color(int x);
void Login(ACC *acc);
void Add_User(ACC *acc);
void Init_Json(char filename[],cJSON**root);
void Select_Dictionary(char filename[]);
void Load_Count(ACC *acc);
void Save_Count(ACC *acc);
void Write_Json();
int main()
{
    WORDS word={};
    ACC acc={acc.count=0};
    char filename[24]="words/";
    Select_Dictionary(filename);
    Init_Json("account/users.json",&acc_root);
    Login(&acc);
    RandomWords(&word,filename);
    int COL=strlen(word.words);
    char arr[ROW][COL+1];
    memset(arr,0,sizeof(arr));
    Game(&word,COL,arr,&acc);
    return 0;
}

//初始化表格并输出（后面的PRINT也是它（）只是使用了宏定义
void Init(int COL,char arr[][COL+1],int count[][COL])
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
    int i,j,count1=0,count2=0;
    for(i=0;i<ROW*2+1;i++)//i行
    {
        for(j=0;j<COL*2+1;j++)//j列
        {
            if(((j%2!=0)&&(i==0||i%2==0)))
            printf("___");
            else if((j==0||j%2==0)&&i!=0)
            printf("|");
            else 
            {
                if(i==0)
                printf(" ");
                else 
                if(arr[count1][count2]!=0)
                {
                    Color(count[count1][count2]);
                    printf(" %c ",arr[count1][count2]);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
                    count2++;
                }
                else
                    printf("   ");
            }
        }
        if(count2==COL)
        {
            count2=0;
            count1++;
        }
        printf("\n");
    }
}

//使用cJSON库从json文件中随机获取单词及释义
void RandomWords(WORDS *words,char filename[])
{
    Init_Json(filename,&words_root);
    cJSON*cur=NULL;
    int count=cJSON_GetArraySize(words_root);//获取单词的数量
    srand((unsigned)time(NULL));
    cur=cJSON_GetArrayItem(words_root,rand()%count);//随机获取一个键
    strcpy(words->words,cur->string);
    strcpy(words->Chinese_meaning ,cJSON_GetObjectItem(cur, "中释")->valuestring);
    cJSON*en=cJSON_GetObjectItem(cur, "英释");
    if(en==NULL)
    strcpy(words->English_meaning,"None");
    else
    strcpy(words->English_meaning ,en->valuestring);
    cJSON_Delete(words_root);//删除words_root对象
    words_root=NULL;
}

//游戏开始
void Game(WORDS* words,int COL,char arr[][COL+1],ACC *acc)
{
    char ch;
    int count[COL][COL],result;
    memset(count,-1,sizeof(count));//初始化count内元素为-1
    printf("%s\n",words->words);
    Init(COL,arr,count);
    printf("猜单词，如上图,单词长度为:%d\n",strlen(words->words));
    printf("每次输入都可以有获得提示的机会,需要提示请输入#,不需要提示随便输一个字母(不显示，不需回车)(共5次机会)\n");
    for(int i=0;i<5;i++)
    {
        
        printf("第%d次：",i+1);
        ch=_getch();
        switch(ch)
        {
            case '#':
            {
                printf("中译：%s\n英译：%s\n",words->Chinese_meaning,words->English_meaning);
                i--;
                break;
            }
            default:
            {
                char c[20];
                printf("\n请输入单词:");
                scanf("%s",c);
                if(strlen(c)>COL)//判断输入是否越界
                c[COL]='\0';
                strcpy(arr[i],c);
                result=Words_cmp(words,COL,arr[i],count[i]);
                PRINT(COL,arr,count);
                break;
            }
        
        }
        if(result==1)
        {
            printf("恭喜猜中\n");
            acc->count++;
            Save_Count(acc);
            break;
        }
        else if(result==0&&i==4)
        {
            printf("游戏结束\n");
            break;
        }
    }
    printf("单词为:%s\n中释为:%s\n英释为:%s",words->words,words->Chinese_meaning,words->English_meaning);
}

//比较字符是否完全正确或部分正确
int Words_cmp(WORDS* words,int COL,char arr[],int count[])
{
    int i,j,flag=0;
    if(strcmp(arr,words->words)==0)
    {
        memset(count,0,4*COL);//count内元素全部赋值为0
        return 1;
    }
    for(i=0;i<COL;i++)
    for(j=0;j<COL;j++)
    {
        if(arr[i]==words->words[i])//字符相同且位置相同
        {
            count[i]=0;
            break;
        }
        if(arr[i]==words->words[j])//字符相同但位置不同
        {
            count[i]=1;
        }
    }
    return 0;
}

//改变输出文字的颜色
void Color(int x)
{
    if(x==1)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),6);//设置为黄色
    else if(x==0)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),2);//设置为绿色
    else if(x==-1)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);//设置为黑色

}

//解析json对象
void Init_Json(char filename[],cJSON**root)
{
    FILE*fp=fopen(filename,"r");
    fseek(fp, 0L, SEEK_END);//指向结尾
    long size = ftell(fp);//获取长度
    fseek(fp, 0L, SEEK_SET);
    char *json =(char*)malloc(size + 1);//给size+1个长度，最后用来存"\0"
    fread(json, 1, size, fp);//将fp文件内容读入到json内
    json[size] = '\0';
    fclose(fp);
    *root= cJSON_Parse(json);// 将JSON数据解析为cJSON对象
    free(json);//释放内存
    json=NULL;
}

//登录账户
void Login(ACC *acc)
{
    int flags = 0;
    cJSON*cur=NULL;
    printf("请输入你的用户名：");
    scanf("%s",acc->name);
    cur=cJSON_GetObjectItem(acc_root,acc->name);
    if(cur==NULL)//查找user.json中是否有与name字符串相等的键
        {
            printf("未找到账号，将为您创建\n");
            flags=1;
        }
    printf("请输入密码：");
    scanf("%s",acc->password);
    if(flags==1)
    {
        Add_User(acc);
    }
    else 
    {
        while(1)
        {
            if(strcmp(acc->password,cJSON_GetObjectItem(cJSON_GetObjectItem(acc_root, acc->name),"password")->valuestring)==0)//比较账户名键对应的password的值与ACC结构体里的值
                {
                    printf("登录成功\n");
                    Load_Count(acc);
                    printf("用户名：%s\n挑战成功次数：%d\n",acc->name,acc->count);
                    break;
                }
            else 
            {
                printf("再次输入或者结束(ctrl+c)\n");
                scanf("%s",acc->password);
            }
        }
    }
    
}

//添加账户
void Add_User(ACC *acc)
{
    char c;
    // 创建一个cJSON对象
    cJSON*root = cJSON_CreateObject();
    // 添加一个以name成员的值为键的嵌套对象
    cJSON *user= cJSON_CreateObject();
    cJSON_AddItemToObject(root, acc->name, user);
    cJSON_AddStringToObject(user,"password",acc->password);
    cJSON_AddNumberToObject(user,"count",acc->count);
    char*json=NULL;
    FILE*fp=fopen("account/users.json","r+");
    fseek(fp, 0L, SEEK_END);//指向结尾
    long size = ftell(fp);//获取长度
    json=cJSON_Print(root);
    if(size!=0)
    {
        fseek(fp,-1, SEEK_END);
        json[0]=',';
    }
    fputs(json,fp);
    free(json);
    printf("用户%s已经被保存",acc->name);
    fclose(fp);
    cJSON_Delete(root);
}

//读取账户猜单词成功次数
void Load_Count(ACC *acc)
{
    cJSON*root=cJSON_GetObjectItem(acc_root,acc->name);
    cJSON*count=cJSON_GetObjectItem(root,"count");
    acc->count=count->valueint;
}

//保存账户成功次数
void Save_Count(ACC *acc)
{
    cJSON*root=cJSON_GetObjectItem(acc_root,acc->name);
    cJSON*count=cJSON_GetObjectItem(root,"count");
    cJSON_SetNumberValue(count,acc->count);
    Write_Json();
}

//选择词典
void Select_Dictionary(char filename[])
{
    printf("需要切换词典吗？默认为CET-4(y/n):");
    char ch=getchar();
    if(ch=='y')
    {
        int a=0;
        char type[10][16]={"CET4.json","CET6.json","GMAT.json","GRE.json","IELTS.json",
                            "SAT.json","TOEFL.json","GEE.json","TEM8.json","TEM4.json"};
        printf("选择词典(输入序号):\n");
        for(int i=0; i<10; i++)
        {
            printf("%d.%s\n",i+1,type[i]);
        }
        scanf("%d",&a);
        strcat(filename,type[a-1]);
    }
    else strcat(filename,"CET4.json");
}

//写入数据
void Write_Json()
{
    FILE*fp=fopen("account/users.json","w");
    char* json=cJSON_Print(acc_root);
    fputs(json,fp);
    free(json);
    cJSON_Delete(acc_root);
}