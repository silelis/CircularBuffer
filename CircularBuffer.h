
/*
 * CircularBuffer.h
 *
 *  Created on: Nov 9, 2020
 *      Author: Dawid "SieliS" Bańkowski
 *      based on: //https://github.com/embeddedartistry/embedded-resources
 *
 */

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_


#include <cstdio>
#include <cstring>

using namespace std;

template<typename T, size_t Size> class CircularBuffer {

public:
	CircularBuffer(void);
	~CircularBuffer(void);
	void put(T item);								//wpisuje dane do bufora kołowego i uaktualnia zmienną head_
	T get(void);									//podaje warość [] z bufora kołowego i uaktualnia zmienną tail_
	bool isEmpty(void) const;
	bool isFull(void) const;
	bool isOverflowed(void) const;
	bool /*int*/ searchItem(const T* item, size_t items, bool tailPosUpdate = true);	//sprawdza czy w buforze kołowym znajdują się dane, jeśli tak to zwraca TRUE i jeśli tailPosUpdate==True to ustawia tail_ w mniejscu pozwalającym na czytanie tej informacji, jeśli bool tailPosUpdate!=true to zwraca tylko informację.
	size_t capacity(void) const;					//zwraca pojemnośc bufora kołowego
	size_t dataSize(void) const;					//zwraca informację na temay ilości danych jakie są aktualnie przechowywane w buforze kołowym
	//todo: return string
	void reset(void);
	void clearBuffer(void);							//Set buffer to NULL do not change head_ and tail_

protected:

private:
	size_t head_ = 0;					//miejsce ostatniego zapisu danych
	size_t tail_ = 0;					//miejsce ostatniego odczytu danych
	size_t max_size_ = Size;			// = Size;
	bool overflow = false;
	bool full_ = 0;
	T buf_[Size];
};


/*==================================================================================================*/

template<typename T, size_t Size> CircularBuffer<T, Size>::CircularBuffer(void) {
	clearBuffer();
}

template<typename T, size_t Size> CircularBuffer<T, Size>::~CircularBuffer(void) {
	head_ = 0;
	tail_ = 0;
	max_size_ = Size;
}

template <typename T, size_t Size> void CircularBuffer<T, Size>::reset(void) {
	head_ = tail_;
	full_ = false;
	overflow = false;
	clearBuffer();
}

template <typename T, size_t Size> void CircularBuffer<T, Size>::clearBuffer(void) {
	memset(buf_, NULL, sizeof(T)*Size);
}

template<typename T, size_t Size> bool  CircularBuffer<T, Size>::isEmpty(void) const {			//if there is no data
	//if head and tail are equal, we are empty
	return (!full_ && (head_ == tail_));
}


template<typename T, size_t Size> bool  CircularBuffer<T, Size>::isFull(void) const {		//dataSize == max_size_
	//If tail is ahead the head by 1, we are full
	return full_;
}


template<typename T, size_t Size> bool CircularBuffer<T, Size>::isOverflowed(void) const {
	return overflow; //(full_ && (head_ == tail_));
}


template<typename T, size_t Size> size_t CircularBuffer<T, Size>::capacity(void) const {
	return max_size_;
}

template<typename T, size_t Size> size_t CircularBuffer<T, Size>::dataSize(void) const {			//freeSpace

	size_t size = max_size_;
	if (!full_) {
		if (head_ >= tail_) {
			return size = head_ - tail_;	//ok
		} else {
			return size = max_size_ + head_ - tail_;
		}
	}

	return size;
}

template<typename T, size_t Size> void CircularBuffer<T, Size>::put(T item) {				//put item into buffer
	//todo: check freespace i zwrot wartościo

	buf_[head_] = item;

	if (full_) {
		tail_ = (tail_ + 1) % max_size_;
		overflow = true;
	}

	head_ = (head_ + 1) % max_size_;
	full_ = head_ == tail_;
}


template<typename T, size_t Size> T CircularBuffer<T, Size>::get(void) {					//read item from buffer
	if (isEmpty()) {
		return T();
	}
	//Read data and advance the tail (we now have a free space)
	/*auto*/ T val = buf_[tail_];
	full_ = false;
	tail_ = (tail_ + 1) % max_size_;

	return val;
}


template<typename T, size_t Size> bool CircularBuffer<T, Size>::searchItem(const T *item, size_t items, bool tailPosUpdate) {
	//szuka czy w buforze znajduje się item[] jeśli tak to:
	//tail_ ustawia na miejsce gdzie znajduje się początek szukanej wartości,
	//ZWRACA:	TRUE	- jeśli odnaleziono szukaną wartość
	//			FALSE	- jeśli nie odnaleziono szukanej wartości


	if (!isOverflowed())
	{
		size_t searchingPosition = tail_;
		size_t toLinearedHead_ = tail_ + dataSize();				//virtualny koniec bufora, gdyby był liniowym
		size_t itemsOnTheEnd;										//liczba pamięta ile danych memcmp powinno sprawdzić na końcu bufora kołowego, a ile na jego początku

		while(searchingPosition+items <= toLinearedHead_)
		{
			searchingPosition = (searchingPosition) % max_size_;

			itemsOnTheEnd = max_size_- searchingPosition;			//sprawdzanie czy części danych nie trzeba szukać na początku bufora kołowego
			if (itemsOnTheEnd>=items)
				itemsOnTheEnd=items;

			if (memcmp((buf_+searchingPosition ), item, sizeof (T)*itemsOnTheEnd)==0)				//sprawdzanie danych na końcu bufora kołowego
			{
				if (memcmp((buf_), item+itemsOnTheEnd, sizeof (T)*(items-itemsOnTheEnd))==0)		//sprawdzanie danych na początku bufora kołowego
				{
					if (tailPosUpdate==true)
						tail_=searchingPosition;		//ustawnienie pozycji odczytu na miejsce taila
					return true;					//znaleziono poprawną sekwencję
				}
			}
			searchingPosition++;
			if (searchingPosition == tail_)		//aby wyjśc z pętli nieskończonej jesli w buforze nie ma szukanego ciągu
				break;
		};
	}
	return false;		//szukany ciąg nazków jest dłuższy niż dane w buforze lub
						//nie znaleziono szukanej sekwencji
						//bufor został przepełniony i istnieje ryzyko błędnego odczytu
}

#endif /* CIRCULARBUFFER_H_ */
