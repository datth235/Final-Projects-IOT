#include<bits/stdc++.h>
using namespace std;
#define long long ll

template<class T>
class Array{
private:
	T* arr; //con tro tro den vung nho luu cac phan tu
	int n;
public:
	Array(){
		arr=NULL;
		n=0;
	}
	Array(T arr[], int n){
		//xin cap phat moi cho arr
		this->arr=new T[n];
		this->n=n;
		//thuc hien copy lan luot tung phan tu cua a sang arr
		for(int i=0;i<n;i++){
			this->arr[i]=arr[i];
		}
	}
	Array(const Array& other){
		this->n=other.n;
		this->arr=new T[this->n];
		for(int i=0;i<n;i++){
			this->arr[i]=other.arr[i];
		}
	}
	~Array(){
		if(arr!=NULL) delete[] arr;
	}
	int getN(){
		return n;
	}
	
	T getTong();
	
	friend void sort(Array<T>* a);
};

template<class T>
T Array<T>::getTong(){
	T res=0;
	for(int i=0;i<n;i++){
		res+=arr[i];
	}
	return res;
}

template<class T>
void sort(Array<T>* a){
	for(int i=0;i<a.n-1;i++){
		int min_pos=i;
		for(int j=i+1;j<a.n;j++){
			if(a.arr[j]<a.arr[min_pos]) min_pos=j;
		}
		int tmp=a.arr[i];
		a.arr[i]=a.arr[min_pos];
		a.arr[min_pos]=tmp;
	}
}

int main(){
 ios_base::sync_with_stdio(false);
 cin.tie(NULL);
 int aa[]={1,2,3,4,5};
 int N=sizeof(aa)/sizeof(aa[0]);
 Array<int> a(aa,N);
 cout<< "Tong cua mang dong a = " << a.getTong()<< endl;
 Array<int> b(a);
 cout<< "Tong cua mang dong b = "<< b.getTong();
}
