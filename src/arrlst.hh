#ifndef _arrlst_hh_
#define _arrlst_hh_

typedef double Type;

template <class Type>
class listitem {
friend class list<Type>;
friend ostream& operator <<( ostream& , const list<Type>& );
protected: 
  listitem(const Type& value=0,
	   listitem<Type> *pred=0,
	   listitem<Type> *succ=0)
    {
      contents=value;
      prev=pred;
      next=succ;
    }
  
  Type contents;
  listitem<Type>	*next, *prev;
};

template <class Type>
class list {
friend ostream& operator <<( ostream& , const list<Type>& );
public:
  list()		{
    start=end=current=0;
    position=length=0;
  }
  
  list(const list<Type>& li)
    {
     start=end=current=0;
      position=length=0;
      *this=li;
    }
  
  ~list()		{
    clear();
  }
  
  inline int	len() const	{return length;}	
  inline int	pos() const	{return position;}
  inline int	isEmpty() const	{return start==0;}
  inline int	isLast() const	{return end==current;}	
  inline int	isFirst() const	{return start==current;}	
  inline int	isValid() const	{return 0!=current;}
  
  inline Type&	value() const;
  inline		operator Type() const {return value(); }
  list<Type>&	operator=(const list<Type>& );
  
  list<Type>&	first();
  list<Type>&	last();
  list<Type>&	locate(const int=0);
  list<Type>&	prev();
  list<Type>&	next();
  list<Type>&	prev(const int distance) {
      for (int i=1; i<=distance; ++i) prev();
      return *this;
    }
  list<Type>&	next(const int distance) {
    for (int i=1; i<=distance; ++i) next();
    return *this;
  }
  
//  list<Type>&	remove();
  list<Type>&	clear();
  
  list<Type>&	append(const Type&);
  list<Type>&	insert(const Type&); 
  list<Type>&	append(const Type& item, const int destination) {
    locate(destination);
    append(item);
  }
  list<Type>&	insert(const Type& item, const int destination) {
    locate(destination);
    append(item);
  }
  list<Type>&	remove(int const destination=0); // {
  //    locate(destination);
  //    remove();
  //    }
  
  
private:
  int position,length;
	listitem<Type> *start,*end,*current;
  
};




template <class Type>
list<Type>& list<Type>::first()
{
  if ( current=start ) 
    position=1;
  else
    position=0;
  return *this;
}

template <class Type>
list<Type>& list<Type>::last()
{
	current=end;
	position=length;
	return *this;
}

template <class Type>
list<Type>& list<Type>::prev()
{
	current=current->prev;
	--position;

	return *this;
}

template <class Type>
list<Type>& list<Type>::next()
{
	if ( current=current->next )
		++position;
	else
		position=0;
	return *this;
}

template <class Type>
Type& list<Type>::value() const
{
	return current->contents;
}

template <class Type>
list<Type>& list<Type>::append(const Type& item)
{
  if (! isEmpty() )
    // target nonnull --> link element into list
    {
      // ok. List contains data, append after current.
      
      
      listitem<Type>
	*source= new listitem<Type>(item, current, current->next );
      
      current->next ? current->next->prev = source : end=source ;
      current->next = source;
      
      current=source;
      ++position;
      ++length;
    }
  else	// list is still empty. Create first item.
    {
      listitem<Type> *source= new listitem<Type>(item, 0, 0 );
      current=start=end=source ;
      position=length=1;
    }
  return *this;
}

template <class Type>
list<Type>& list<Type>::insert(const Type& item)
{
  // set current and position to the selected value
  // if the list is still empty, null is return, otherwise
  // current.
  
  if (! isEmpty())
    {
      // ok. List contains data, insert in front of dest
      
      listitem<Type>
	*source= new listitem<Type>(item, current->prev, current );
      
      current->prev ? current->prev->next = source : start=source ;
      current->prev = source;
      ++length;
      current=source;
    }
  else	// list is still empty. Create first item.
    {
      listitem<Type> *source= new listitem<Type>(item, 0, 0 );
      current=start=end=source ;
      position=length=1;
    }
  return *this;
}

template <class Type>
list<Type>& list<Type>::locate(const int dest)
{
  if ( dest != 0 )
    {
      first();
      for (int i=1; i<dest ; ++i) next();
    }
  return *this;
}

template <class Type>
list<Type>& list<Type>::remove(int const destination=0) {
  if (destination) locate(destination);
  listitem<Type> *tmpptr;
  tmpptr=current;
  
  // unlink tmpptr from list
  tmpptr->next ? tmpptr->next->prev=tmpptr->prev : end=tmpptr->prev;
  tmpptr->prev ? tmpptr->prev->next=tmpptr->next : start=tmpptr->next;
  
  // report list to be one element shorter
  --length;
  
  // yes. One item beyond this left?
  if ( 0 != tmpptr->next ) 
    //yes. So that's the new current pointer.
    current=tmpptr->next;
  else
    {
      //nope. Take the one in front of this item.
      //decrement position.
      current=tmpptr->prev;
      --position;
      // this also handles removal of last item
      // current is then 0, the same for position
    }
  
  // now the item may be removed (it is unlinked and counts are corrected)
  delete tmpptr;
  return *this;
}

template <class Type>
list<Type>& list<Type>::operator=( const list<Type>& li )
{
  announce("list<>::operator=( list<>& )");
  if (this == &li) return *this;	//source and target identical
  
  //clear target
  clear();
  
  if ( ! li.isEmpty() )
    {
      /*	illegal since li is declared const. :-(
       *		// save pos in source
       *		int savedpos=li.pos();
       *
       *		// li is *not* empty, so there must be a first element!
       *		append(li.first().value());	
       *
       *		// now go on for any following element
       *		while ( ! li.isLast())
       *		{
       *			append(li.next().value());
       *		}
       *
       *		// reset current to first
       *		first();
       *
       *		// restore li.current
       *		li.locate(savedpos);			*/
      listitem<Type> *tmp_ptr;
      tmp_ptr=li.start;
      while (0!=tmp_ptr)
	{
	  append(tmp_ptr->contents);
	  tmp_ptr=tmp_ptr->next;
	}
    }
  return *this;	
  
}

template <class Type>
list<Type>& list<Type>::clear()
{
  if (! isEmpty() )
    { 
      first();
      while ( ! isEmpty() ) remove() ;
    }
  return *this;
}

template <class Type>
ostream& operator <<( ostream& os, const list<Type>& li )
{
  /*	// save current position
   *	int p=li.pos();
   *
   *	// output list contents:
   *
   *	os << "< (" << li.len() << ") " ;
   *
   *	if (!li.isEmpty())
   *	for ( os << li.first().value() << " " ; !li.isLast() ;)
   *	{
   *		os << li.next().value() << " " ;
   *	}
   *
   *	os << ">";
   *
   *	// restore current position
   *	if (!li.isEmpty()) li.locate(p);		*/
  
  listitem<Type>	*tmp_ptr;
  tmp_ptr=li.start;
  
  // output list contents:
  
  os << "< (" << li.len() << ") " ;
  
  while	(0!=tmp_ptr)
    {
      os << tmp_ptr->contents << " " ;
      tmp_ptr=tmp_ptr->next;
    }
  
  os << ">";
  
	return os;
}
// arrlst
#include <values.h>

template <class Type>
class element_cluster {
friend class arrlst<Type>;
public: 
  // the defaults are to create a dummy.
  // it is never really called without parameters.
  // (list does---but assignes sth immediately)
  element_cluster() {offset = 0; cluster = new Type[size=0];}
  element_cluster(int off, Type dflt, int siz)
    {
      offset=off;
      cluster=new Type[size=siz];
      for (register int i=0; i<size; ++i)
	cluster[i]=dflt;
    }
  ~element_cluster()
    {
      delete [] cluster;	
    }
  element_cluster<Type>&
    operator=(const element_cluster<Type> src)
      {
	delete [] cluster;
	cluster = new Type[size=src.size];
	
	for (register int i=0; i<size; i++)
	  cluster[i]=src.cluster[i];
	offset=src.offset;
      }
protected: 
  int	offset,size;
  Type	*cluster;
};


template <class Type>
class arrlst{
private: 
  void arrlst_Default_Value(Type &dummy) {}
  void arrlst_Default_Value(int &d) {d = 0;}
  void arrlst_Default_Value(long int &d) {d = 0;}
  void arrlst_Default_Value(short int &d) {d = 0;}
  void arrlst_Default_Value(double &d) {d = 0.0;}
  void arrlst_Default_Value(float &d) {d = 0.0;}
  void arrlst_Default_Value(char* &d) {d = 0;}
  void arrlst_Default_Value(void* &d) {d = 0;}
public: 
  arrlst() {
    clustersize = 100; 
    arrlst_Default_Value(def_value);
    maxsize = MAXINT;
  } 
  arrlst( Type dflt, int clsz=100, int maxsiz=MAXINT )
    {
      clustersize=clsz;
      def_value=dflt;
      maxsize=maxsiz;
    }
  ~arrlst()
    {
      li.clear();
    }
  Type&	operator[](int);
  
private: 
  list< element_cluster<Type> >		li;
  int			clustersize,maxsize;
  Type			def_value;
};

template <class Type>
Type& arrlst<Type>::operator[](int i)
{
  if (li.isEmpty())
    {
      // not yet there
      element_cluster<Type>
	tmp_cluster( i-(i % clustersize),
		    def_value,
		    clustersize);
      
      li.append(tmp_cluster);
    }
  else
    {
      if (i>=li.value().offset) 
	{
	  //i is in or after current cluster.
	  if  (i<li.value().offset + clustersize)
	    {
	      // i is in current cluster
	    }
	  else
	    {
	      // i is after current cluster.
	      
	      while ((!li.isLast()) && (i>=li.value().offset+clustersize))
		{
		  li.next();
		}
	      
	      if ((i>=li.value().offset) &&
		  (i <li.value().offset + clustersize))
		{
		  // we've found it
		  // i is now in the current cluster
		} 
	      else 
		{
		  // not yet there
		  element_cluster<Type>
		    tmp_cluster( i-(i % clustersize),
				def_value,
				clustersize);
		  if (i<li.value().offset)
		    {
		      // insert in front
		      li.insert(tmp_cluster);
		    }
		  else
		    {
		      // must be last element
		      li.append(tmp_cluster);
		    }
		  // ok. now it's there.
		  // i is in the current cluster.
		}
	    }
	}
      else
	{
	  // i is in front of current cluster.
	  while ((!li.isFirst()) && (i<li.value().offset))
	    {
	      li.prev();
	    }
	  
	  if ((i>=li.value().offset) &&
	      (i <li.value().offset + clustersize))
	    {
	      // we've found it
	      // i is now in the current cluster
	    }
	  else
	    {
	      // not yet there
	      
	      element_cluster<Type>
		tmp_cluster( i-(i % clustersize),
			    def_value,
			    clustersize);
	      
	      if (i<li.value().offset)
		{
		  // must be first item...
		  // insert in front
		  li.insert(tmp_cluster);
		}
	      else
		{
		  li.append(tmp_cluster);
		}
	      // ok. now it's there.
	      // i is in the current cluster.
	    }
	}
    }
  // it is either found or created now, return it!
  return li.value().cluster[i-li.value().offset];
}

#endif
