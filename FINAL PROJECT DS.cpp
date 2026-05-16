#include <cstdio>
#include <cstring>
#include <cctype>

using namespace std;

const int MAXN = 500000;   
const int MAXWORDS = 10000;


void cleanString(char* s) {
    int i = 0, j = 0;
    while (s[i]) {
        if (isalpha(s[i])) s[j++] = tolower(s[i]);
        else s[j++] = ' '; 
        
        i++;
    }
    s[j] = 0;
}

int minInt(int a, int b){ return a<b?a:b; }
int maxInt(int a, int b){ return a>b?a:b; }

// ----------------- Suffix Array -----------------
char S[2*MAXN];
int SA[2*MAXN], tempSA[2*MAXN];
int RA[2*MAXN], tempRA[2*MAXN];
int LCP[2*MAXN], invSA[2*MAXN];
int N;

void countingSort(int k){
    int maxi = maxInt(300,N);
    int* c = new int[maxi];
    memset(c,0,sizeof(int)*maxi);
    for(int i=0;i<N;i++) c[(i+k<N)?RA[i+k]:0]++;
    for(int i=1;i<maxi;i++) c[i]+=c[i-1];
    for(int i=N-1;i>=0;i--){
        int idx = (SA[i]+k<N)?RA[SA[i]+k]:0;
        tempSA[--c[idx]] = SA[i];
    }
    for(int i=0;i<N;i++) SA[i]=tempSA[i];
    delete[] c;
}

void buildSA(){
    for(int i=0;i<N;i++){ SA[i]=i; RA[i]=S[i]; }
    for(int k=1,r;k<N;k<<=1){
        countingSort(k); countingSort(0);
        tempRA[SA[0]]=r=0;
        for(int i=1;i<N;i++){
            bool same = (RA[SA[i]]==RA[SA[i-1]] &&
                        ((SA[i]+k<N?RA[SA[i]+k]:0)==(SA[i-1]+k<N?RA[SA[i-1]+k]:0)));
            tempRA[SA[i]] = same?r:++r;
        }
        for(int i=0;i<N;i++) RA[i]=tempRA[i];
        if(RA[SA[N-1]]==N-1) break;
    }
}

void buildLCP(){
    int h=0;
    for(int i=0;i<N;i++) invSA[SA[i]]=i;
    for(int i=0;i<N;i++){
        if(invSA[i]==0){ LCP[0]=0; continue; }
        int j=SA[invSA[i]-1];
        while(i+h<N && j+h<N && S[i+h]==S[j+h]) h++;
        LCP[invSA[i]]=h;
        if(h>0) h--;
    }
}

int longestCommonSubstring(const char *a, const char *b){
    int len1 = strlen(a);
    int len2 = strlen(b);
    for(int i=0;i<len1;i++) S[i]=a[i];
    S[len1]=1; // separator
    for(int i=0;i<len2;i++) S[len1+1+i]=b[i];
    N = len1 + len2 + 1;
    buildSA();
    buildLCP();
    int ans=0;
    for(int i=1;i<N;i++){
        bool left=SA[i]<len1, right=SA[i-1]<len1;
        if(left!=right) ans=maxInt(ans,LCP[i]);
    }
    return ans;
}

// ----------------- Rabin-Karp -----------------
bool rabinKarpSub(const char *pattern,const char *text){
    int m=strlen(pattern), n=strlen(text);
    if(m>n) return false;
    const long long MOD=1000000007, BASE=31;
    long long patHash=0, winHash=0, power=1;
    for(int i=m-1;i>=0;i--){
        patHash=(patHash + (pattern[i]-'a'+1)*power)%MOD;
        winHash=(winHash + (text[i]-'a'+1)*power)%MOD;
        if(i>0) power=(power*BASE)%MOD;
    }
    power=1;
    for(int i=0;i<=n-m;i++){
        if(patHash==winHash){
            bool ok=true;
            for(int j=0;j<m;j++) if(text[i+j]!=pattern[j]){ ok=false; break; }
            if(ok) return true;
        }
        if(i<n-m){
            int left=text[i]-'a'+1, right=text[i+m]-'a'+1;
            winHash=(winHash - left*power%MOD + MOD)%MOD;
            winHash=(winHash*BASE + right)%MOD;
        }
    }
    return false;
}

int sub10count(const char *a,const char *b){
    int len=strlen(a);
    if(len<10) return 0;
    char s[11]; 
    for(int i=0;i<10;i++) s[i]=a[i];
    s[10]=0;
    return rabinKarpSub(s,b)?1:0;
}

// ----------------- Prefix -----------------
int prefixLength(const char *a,const char *b){
    int len = minInt(strlen(a), strlen(b));
    for(int i=0;i<len;i++) if(a[i]!=b[i]) return i;
    return len;
}

// ----------------- Trie Inverted Index -----------------
struct TrieNode{
    TrieNode* children[26];
    int docs[MAXWORDS], docCount;
    TrieNode(){ docCount=0; for(int i=0;i<26;i++) children[i]=nullptr; }
};

class TrieInvertedIndex{
    TrieNode* root;
public:
    TrieInvertedIndex(){ root=new TrieNode(); }
    void insertWord(const char *word,int docID){
        TrieNode* node=root;
        for(int i=0;word[i];i++){
            char c=word[i];
            if(c<'a'||c>'z') continue;
            int idx=c-'a';
            if(!node->children[idx]) node->children[idx]=new TrieNode();
            node=node->children[idx];
        }
        node->docs[node->docCount++]=docID;
    }
    void addDocument(const char *text,int docID){
        char w[1000]; int wi=0;
        for(int i=0;text[i];i++){
            char c=tolower(text[i]);
            if(isalpha(c)) w[wi++]=c;
            else if(wi>0){ w[wi]=0; insertWord(w,docID); wi=0; }
        }
        if(wi>0){ w[wi]=0; insertWord(w,docID); }
    }
    int commonWords(const char *a,const char *b){
        char wa[MAXWORDS][50], wb[MAXWORDS][50];
        int na=0, nb=0;
        char w[50]; int wi=0;
        for(int i=0;a[i];i++){
            char c=tolower(a[i]);
            if(isalpha(c)) w[wi++]=c;
            else if(wi>0){ w[wi]=0; strcpy(wa[na++],w); wi=0; }
        }
        if(wi>0){ w[wi]=0; strcpy(wa[na++],w); }
        wi=0;
        for(int i=0;b[i];i++){
            char c=tolower(b[i]);
            if(isalpha(c)) w[wi++]=c;
            else if(wi>0){ w[wi]=0; strcpy(wb[nb++],w); wi=0; }
        }
        if(wi>0){ w[wi]=0; strcpy(wb[nb++],w); }
        int cnt=0;
        for(int i=0;i<na;i++){
            for(int j=0;j<nb;j++){
                if(strcmp(wa[i],wb[j])==0){ cnt++; break; }
            }
        }
        return cnt;
    }
};

// ----------------- Heap Feature -----------------
struct MaxHeap{
    int data[MAXWORDS], size;
    void init(){ size=0; }
    void push(int v){
        data[size]=v; int i=size; size++;
        while(i>0){
            int p=(i-1)/2;
            if(data[p]>=data[i]) break;
            int tmp=data[p]; data[p]=data[i]; data[i]=tmp;
            i=p;
        }
    }
    int pop(){
        if(size==0) return -1;
        int ret=data[0];
        data[0]=data[--size];
        int i=0;
        while(true){
            int l=2*i+1, r=2*i+2, largest=i;
            if(l<size && data[l]>data[largest]) largest=l;
            if(r<size && data[r]>data[largest]) largest=r;
            if(largest==i) break;
            int tmp=data[i]; data[i]=data[largest]; data[largest]=tmp;
            i=largest;
        }
        return ret;
    }
};

int heapFeature(const char *a,const char *b){
    char wa[MAXWORDS][50], wb[MAXWORDS][50];
    int na=0, nb=0;
    char w[50]; int wi=0;
    for(int i=0;a[i];i++){
        char c=tolower(a[i]);
        if(isalpha(c)) w[wi++]=c;
        else if(wi>0){ w[wi]=0; strcpy(wa[na++],w); wi=0; }
    }
    if(wi>0){ w[wi]=0; strcpy(wa[na++],w); }
    wi=0;
    for(int i=0;b[i];i++){
        char c=tolower(b[i]);
        if(isalpha(c)) w[wi++]=c;
        else if(wi>0){ w[wi]=0; strcpy(wb[nb++],w); wi=0; }
    }
    if(wi>0){ w[wi]=0; strcpy(wb[nb++],w); }

    MaxHeap heap; heap.init();
    for(int i=0;i<na;i++){
        for(int j=0;j<nb;j++){
            if(strcmp(wa[i],wb[j])==0){ heap.push(strlen(wa[i])); break; }
        }
    }
    int sum=0;
    for(int i=0;i<3;i++){
        int v=heap.pop(); if(v==-1) break;
        sum+=v;
    }
    return sum;
}

// ----------------- Safe CSV -----------------
void safeCSV(const char* s,char* out){
    char* p=out;
    *p++='"';
    for(int i=0;s[i];i++){
        if(s[i]=='"'){ *p++='"'; *p++='"'; }
        else if(s[i]=='\n'||s[i]=='\r') *p++=' ';
        else *p++=s[i];
    }
    *p++='"'; *p=0;
}

// ----------------- MAIN -----------------
int main(){
    FILE* fin=fopen("train_snli.txt","r");
    if(!fin){ printf("Cannot open train_snli.txt\n"); return 1; }
    FILE* fout=fopen("features.csv","w");
    if(!fout){ printf("Cannot open features.csv\n"); return 1; }

    fprintf(fout,"text1,text2,prefix,sub10,lcs,inverted,heap,label\n");

    TrieInvertedIndex invTrie;
    int docID=0;
    char line[10000];

    while(fgets(line,sizeof(line),fin)){
        if(strlen(line)<3) continue;
        char* firstTab=strchr(line,'\t');
        if(!firstTab) continue;
        char* secondTab=strchr(firstTab+1,'\t');
        if(!secondTab) continue;
        *firstTab=0; *secondTab=0;

        char* s1=line; char* s2=firstTab+1; char* lbl=secondTab+1;
        lbl[strcspn(lbl,"\r\n")]=0;

        cleanString(s1);
        cleanString(s2);

        int prefix = prefixLength(s1,s2);
        int sub10  = sub10count(s1,s2);
        int lcs    = longestCommonSubstring(s1,s2);
        int invc   = invTrie.commonWords(s1,s2);
        int pqf    = heapFeature(s1,s2);

        char out1[12000], out2[12000];
        safeCSV(s1,out1);
        safeCSV(s2,out2);

        fprintf(fout,"%s,%s,%d,%d,%d,%d,%d,%s\n",
            out1,out2,prefix,sub10,lcs,invc,pqf,lbl);

        invTrie.addDocument(s1,docID++);
        invTrie.addDocument(s2,docID++);
    }

    fclose(fin);
    fclose(fout);
    printf("features.csv generated successfully.\n");
    return 0;
}
