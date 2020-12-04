#ifndef STD_BITS_H
#define STD_BITS_H

// TODO: Error handling
#define SET_BIT(bitfield, bit)                                                 \
	do {                                                                       \
		__typeof__(bitfield) _bit = (bit);                                     \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		(bitfield) |= 1U << _bit;                                              \
	} while (0)

// TODO: Error handling
#define CLEAR_BIT(bitfield, bit)                                               \
	do {                                                                       \
		__typeof__(bitfield) _bit = (bit);                                     \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		(bitfield) &= ~(1U << _bit);                                           \
	} while (0)

// TODO: Error handling
#define SET_BIT_TO(bitfield, bit, value, value_len)                            \
	do {                                                                       \
		__typeof__(bitfield) _bit = (bit);                                     \
		__typeof__(value) _value  = (value);                                   \
		/*
		 * Since `bitfield` should me modified, we have to create a pointer to
		 * 'escape' the local context of the `do while`
		 */                                                                     \
		__typeof__(bitfield)* _bitfield = &(bitfield);                         \
		__typeof__(bitfield) _value_len = (value_len);                         \
		if (_bit > (sizeof(__typeof__(bitfield)) * 8) - 1)                     \
			break;                                                             \
		/* Clear all touched bits before setting the value, see 3b38e6b. */    \
		for (__typeof__(_value_len) i = _bit; i < _bit + _value_len; ++i)      \
			CLEAR_BIT(*_bitfield, i);                                          \
		*_bitfield |= _value << _bit;                                          \
	} while (0)

#define IS_SET(bitfield, bit) (!!((bitfield) & (1U << (bit))))

#endif /* STD_BITS_H */
