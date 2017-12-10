/*30:*/
#line 452 "./iawlb.w"

/*36:*/
#line 543 "./iawlb.w"

#include <list> 

#include <pthread.h> 

/*:36*/
#line 453 "./iawlb.w"

template<typename T> 
class SyncedList
{
private:
pthread_mutex_t m_;
pthread_cond_t c_;
std::list<T> list_;

public:
/*31:*/
#line 470 "./iawlb.w"

SyncedList(){
(void)pthread_mutex_init(&m_,NULL);
(void)pthread_cond_init(&c_,NULL);
}
~SyncedList(){
(void)pthread_mutex_destroy(&m_);
(void)pthread_cond_destroy(&c_);
}

/*:31*/
#line 463 "./iawlb.w"

/*32:*/
#line 492 "./iawlb.w"

bool pop(T*dst){
bool taken= false;
int r= pthread_mutex_lock(&m_);
if(list_.size()==0){
waitdata();
}
if(list_.size()> 0){
*dst= list_.front();
list_.pop_front();
taken= true;
}
r= pthread_mutex_unlock(&m_);
return taken;
}

/*:32*//*33:*/
#line 511 "./iawlb.w"

void enqueue(T&e){
int r= pthread_mutex_lock(&m_);
list_.push_back(e);
wakeup();
r= pthread_mutex_unlock(&m_);
}

/*:33*//*37:*/
#line 577 "./iawlb.w"

void enqueue(std::list<T> &e){
int r= pthread_mutex_lock(&m_);
list_.insert(list_.cbegin(),
e.begin(),e.end());
wakeup();
r= pthread_mutex_unlock(&m_);
}


/*:37*/
#line 464 "./iawlb.w"

/*34:*/
#line 525 "./iawlb.w"

void waitdata(){

while(list_.size()==0){
(void)pthread_cond_wait(&c_,&m_);
}
}

/*:34*//*35:*/
#line 538 "./iawlb.w"

void wakeup(){
(void)pthread_cond_broadcast(&c_);
}

/*:35*/
#line 465 "./iawlb.w"

};

/*:30*/
