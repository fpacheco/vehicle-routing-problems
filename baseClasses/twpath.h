#ifndef TWPATH_H
#define TWPATH_H

#include <deque>
#include <iostream>

/*
    Evaluation has to be done

*/

template <class knode> class Twpath {
  protected:
    std::deque<knode> path;


  public:
    void swap(int i, int j) {
        knode temp = path[i];
        path[i] = path[j];
        path[j] = temp;
    } 

    void move(int fromi, int toj) {
        if (fromi == toj) return;
        if (fromi < toj){
            insert(path[fromi], toj + 1);
            remove(fromi);
        } else {
            insert(path[fromi], toj);
            remove(fromi + 1);
        }
    };

    void insert(const knode &n, int at) { path.insert(path.begin() + at, n); };
    void remove (int i) { path.erase(path.begin()+i); };
    void push_back(const knode& n) { path.push_back(n); };
    void push_back(knode& n) { path.push_back(n); };
    void push_front(const knode& n) { path.push_front(n); };
    void push_front(knode& n) { path.push_front(n); };
    void pop_back() { path.pop_back(); };
    void pop_front() { path.pop_front(); };





/* how can we hande evaluation if we need the following??? */

    void resize(unsigned int n) { path.resize(n); };
    void clear() { path.clear(); };


    public:   
    // Capacity
    unsigned int max_size() const { return path.max_size(); };
    unsigned int size() const { return path.size(); };
    bool empty() const { return path.empty(); };



    /* nodes handling within two  paths */
    void swap(int i, double maxcap, Twpath<knode> &rhs, int j, double rhs_maxcap) {
        knode temp = path[i];
        path[i] = rhs.path[j];
        rhs.path[j] = temp;
        evaluate(i, maxcap);
        rhs.evaluate(j, rhs_maxcap);
    }



    /* nodes handling within the same path */
    void move(int fromi, int toj, double maxcapacity) {
        if (fromi == toj) return;
        if (fromi < toj){
            insert(path[fromi], toj + 1);
            remove(fromi, maxcapacity);
        } else {
            insert(path[fromi], toj);
            remove(fromi + 1, maxcapacity);
        }
    };

    void resize(unsigned int at,double maxcapacity) { 
        path.resize(at);
        //theoricamente funciona sin mas
    };

    void swap(int i,int j,double maxcapacity) {
        if (i==j) return;
        knode temp = path[i];
        path[i] = path[j];
        path[j] = temp;
        i < j ? evaluate(i, maxcapacity): evaluate(j, maxcapacity);
    };

    void insert(const knode &n, int at, double maxcapacity) {
        path.insert(path.begin() + at, n);
        path[at].evaluate(maxcapacity);
    };

    void push_back(const knode& n, double maxcapacity) {
        path.push_back(n);
        evalLast(maxcapacity);
    };

    void push_back(knode& n, double maxcapacity) {
        path.push_back(n);
        evalLast(maxcapacity);
    };

    void remove (int i, double maxcapacity) {
        path.erase(path.begin() + i);
        evaluate(i, maxcapacity);
    };

    /*****   EVALUATION   ****/
    void evaluate(int from,double maxcapacity) {

        if (from < 0 or from > path.size()) from = 0;
        for (int i=from; i<path.size(); i++) {
            if (i == 0) path[0].evaluate(maxcapacity);
            else path[i].evaluate(path[i-1], maxcapacity);
        };
    };

    void evalLast(double maxcapacity) {
        evaluate(path.size()-1, maxcapacity);
    };

    void evaluate(double maxcapacity){
        evaluate(0,maxcapacity);
    };
    
    /*** ACCESSORS ***/

    std::deque<int> getpath() const {
        std::deque<int> p;
        for (int i=0; i<path.size(); i++){
              p.push_back(path[i].getnid());}
        return p;
    };

    void dump() {
        std::cout << "Twpath: "; // << home.getnid();
        for (int i=0; i<path.size(); i++)
            std::cout << ", " << path[i].getnid();
        std::cout << std::endl;
    };

    // element access
    knode& operator[](unsigned int n) { return path[n]; };
    knode  operator[] (unsigned int n) const { return path[n]; };
    knode& at(int n) { return path.at(n); };
    knode at(int n) const  { return path.at(n); };
    knode& front() { return path.front(); };
    knode front() const { return path.front(); };
    knode& back() { return path.back(); };
    knode back() const { return path.back(); };


/*
    //  PATH specific operations

    typedef typename std::deque<knode> nodepath;
    typedef typename std::deque<knode>::iterator iterator;
    typedef typename std::deque<knode>::const_iterator const_iterator;



    // iterators
//    iterator begin() { return path.begin(); };
//    iterator end() { return path.end(); };
//    iterator rbegin() { return path.rbegin(); };
    //iterator rend() { return path.rend(); };
    //const_iterator cbegin() { return path.cbegin(); };
    //const_iterator cend() { return path.cend(); };
    //const_iterator crbegin() { return path.crbegin(); };
    //const_iterator crend() { return path.crend(); };


//iterator insert(iterator it, const knode& n) { return path.insert(it, n); };

    //void shrink_to_fit() { path.shrink_to_fit(); };

    // modifiers
    //iterator insert(iterator it, Trashnode& n) { return path.insert(it, n); };
    //iterator erase(iterator it) { return path.erase(it); };
    //iterator erase(iterator first, iterator last) { return path.erase(first, last); };
    //iterator emplace(const_iterator it, const Trashnode& n) { return path.emplace(it, n); };
    //iterator emplace_front(const Trashnode& n) { return path.emplace_front(n); };
    //iterator emplace_back(const Trashnode& n) { return path.emplace_back(n); };
*/


};


#endif

