#include <cstdio>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <string>

using namespace std;

const int MAXN = 200000;
const int MAXWORDS = 5000;

// ---------------- Suffix Array ----------------
char S[2*MAXN];
int SA[2*MAXN], tempSA[2*MAXN];
int RA[2*MAXN], tempRA[2*MAXN];
int LCP[2*MAXN], invSA[2*MAXN];
int N;

void countingSort(int k){
    int maxi = max(300, N);
    int c[maxi]; memset(c,0,sizeof(c));
    for(int i=0;i<N;i++) c[(i+k<N)?RA[i+k]:0]++;
    for(int i=1;i<maxi;i++) c[i]+=c[i-1];
    for(int i=N-1;i>=0;i--){
        int idx = (SA[i]+k<N)?RA[SA[i]+k]:0;
        tempSA[--c[idx]] = SA[i];
    }
    for(int i=0;i<N;i++) SA[i]=tempSA[i];
}

void buildSA(){
    for(int i=0;i<N;i++){ SA[i]=i; RA[i]=S[i]; }
    for(int k=1,r;k<N;k<<=1){
        countingSort(k); countingSort(0);
        tempRA[SA[0]]=r=0;
        for(int i=1;i<N;i++){
            bool same=(RA[SA[i]]==RA[SA[i-1]] && ((SA[i]+k<N?RA[SA[i]+k]:0)==(SA[i-1]+k<N?RA[SA[i-1]+k]:0)));
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
        LCP[invSA[i]]=h; if(h>0) h--;
    }
}

int longestCommonSubstring(const string &a, const string &b){
    int len1=a.size();
    for(int i=0;i<len1;i++) S[i]=a[i];
    S[len1]=1;
    int len2=b.size();
    for(int i=0;i<len2;i++) S[len1+1+i]=b[i];
    N = len1 + len2 + 1;
    buildSA(); buildLCP();
    int ans=0;
    for(int i=1;i<N;i++){
        bool left=SA[i]<len1, right=SA[i-1]<len1;
        if(left!=right) ans=max(ans,LCP[i]);
    }
    return ans;
}

// ---------------- Rabin-Karp ----------------
bool rabinKarpSub(const string &pattern,const string &text){
    int m=pattern.size(), n=text.size();
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
            winHash=(winHash-left*power%MOD+MOD)%MOD;
            winHash=(winHash*BASE + right)%MOD;
        }
    }
    return false;
}

int sub10count(const string &a,const string &b){
    if(a.size()<10) return 0;
    string s=a.substr(0,10);
    return rabinKarpSub(s,b)?1:0;
}

// ---------------- Prefix ----------------
int prefixLength(const string &a,const string &b){
    int len = min((int)a.size(), (int)b.size());
    for(int i=0;i<len;i++) if(a[i]!=b[i]) return i;
    return len;
}

// ---------------- Trie-based Inverted Index ----------------
struct TrieNode {
    TrieNode* children[26];
    int docs[MAXWORDS];
    int docCount;
    TrieNode(){ docCount=0; for(int i=0;i<26;i++) children[i]=nullptr; }
};

class TrieInvertedIndex {
    TrieNode* root;
public:
    TrieInvertedIndex(){ root=new TrieNode(); }
    
    void insertWord(const string &word,int docID){
        TrieNode* node=root;
        for(char c: word){
            if(c<'a'||c>'z') continue;
            int idx=c-'a';
            if(!node->children[idx]) node->children[idx]=new TrieNode();
            node=node->children[idx];
        }
        node->docs[node->docCount++]=docID;
    }

    void addDocument(const string &text,int docID){
        string w="";
        for(char c: text){
            c=tolower(c);
            if(isalpha(c)) w+=c;
            else if(!w.empty()){ insertWord(w,docID); w=""; }
        }
        if(!w.empty()) insertWord(w,docID);
    }

    int commonWords(const string &a,const string &b){
        // simple comparison using trie search
        string wa[MAXWORDS], wb[MAXWORDS];
        int na=0, nb=0;
        string w="";
        for(char c: a){ c=tolower(c); if(isalpha(c)) w+=c; else if(!w.empty()){ wa[na++]=w; w=""; } }
        if(!w.empty()) wa[na++]=w;
        w="";
        for(char c: b){ c=tolower(c); if(isalpha(c)) w+=c; else if(!w.empty()){ wb[nb++]=w; w=""; } }
        if(!w.empty()) wb[nb++]=w;
        // brute-force check (without STL set/map)
        int cnt=0;
        for(int i=0;i<na;i++){
            for(int j=0;j<nb;j++){
                if(wa[i]==wb[j]){ cnt++; break; }
            }
        }
        return cnt;
    }
};

// ---------------- Heap Feature ----------------
struct MaxHeap{
    int data[MAXWORDS]; int size;
    void init(){ size=0; }
    void push(int v){
        data[size]=v; int i=size; size++;
        while(i>0){ int p=(i-1)/2; if(data[p]>=data[i]) break; swap(data[p],data[i]); i=p; }
    }
    int pop(){
        if(size==0) return -1;
        int ret=data[0]; data[0]=data[--size];
        int i=0;
        while(true){
            int l=2*i+1, r=2*i+2, largest=i;
            if(l<size && data[l]>data[largest]) largest=l;
            if(r<size && data[r]>data[largest]) largest=r;
            if(largest==i) break;
            swap(data[i],data[largest]); i=largest;
        }
        return ret;
    }
};

int heapFeature(const string &a,const string &b){
    string wa[MAXWORDS], wb[MAXWORDS]; int na=0, nb=0;
    string w="";
    for(char c: a){ c=tolower(c); if(isalpha(c)) w+=c; else if(!w.empty()){ wa[na++]=w; w=""; } }
    if(!w.empty()) wa[na++]=w;
    w="";
    for(char c: b){ c=tolower(c); if(isalpha(c)) w+=c; else if(!w.empty()){ wb[nb++]=w; w=""; } }
    if(!w.empty()) wb[nb++]=w;

    MaxHeap heap; heap.init();
    for(int i=0;i<na;i++){
        for(int j=0;j<nb;j++){
            if(wa[i]==wb[j]){ heap.push(wa[i].size()); break; }
        }
    }
    int sum=0;
    for(int i=0;i<3;i++){
        int v=heap.pop(); if(v==-1) break;
        sum+=v;
    }
    return sum;
}

// ---------------- Safe CSV ----------------
string safeCSV(const string &s){
    string out="\"";
    for(char c: s){
        if(c=='"') out+="\"\"";
        else if(c=='\n'||c=='\r') out+=' ';
        else out+=c;
    }
    out+="\"";
    return out;
}

// ---------------- MAIN ----------------
int main(){
    FILE *fin=fopen("input.txt","r");
    if(!fin){ printf("Cannot open input.txt\n"); return 1; }
    FILE *fout=fopen("new_features.csv","w");
    if(!fout){ printf("Cannot open output file\n"); return 1; }

    fprintf(fout,"text1,text2,prefix,sub10,lcs,inverted,heap,label\n");

    TrieInvertedIndex inv;
    char buffer[10000];
    int docID=0;
    while(fgets(buffer,sizeof(buffer),fin)){
        string line=buffer;
        if(line.size()<3) continue;

        size_t tab=line.find('\t');
        string s1, s2;
        if(tab!=string::npos){ s1=line.substr(0,tab); s2=line.substr(tab+1); }
        else { s1=line; s2=line; }

        s2.erase(remove(s2.begin(),s2.end(),'\n'), s2.end());
        s2.erase(remove(s2.begin(),s2.end(),'\r'), s2.end());
        for(char &c:s1) c=tolower(c);
        for(char &c:s2) c=tolower(c);

        int prefix=prefixLength(s1,s2);
        int sub10=sub10count(s1,s2);
        int lcs=longestCommonSubstring(s1,s2);
        int inverted=inv.commonWords(s1,s2);
        int heap=heapFeature(s1,s2);

        fprintf(fout,"%s,%s,%d,%d,%d,%d,%d,0\n",
            safeCSV(s1).c_str(), safeCSV(s2).c_str(), prefix, sub10, lcs, inverted, heap);

        inv.addDocument(s1, docID); inv.addDocument(s2, docID+1); docID+=2;
    }

    fclose(fin); fclose(fout);
    printf("new_features.csv generated successfully.\n");
    return 0;
}
