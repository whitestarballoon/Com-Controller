/*!
 * \file BaseMsg.h
 * Copyright (c) 2012 by dorgby.net
 * \brief 
 *
 * Created: 03/11/2012 17:43:16
 * Revision: $Revision$ 
 * Author: $Author$
 * Last Changed: $Date$
 */

#ifndef __BASE_MSG__
#define __BASE_MSG__

class BaseMsg 
{
        public:
                BaseMsg() { }
                virtual ~BaseMsg() { }

        public:
                virtual int getFormattedLength(void) = 0;
                virtual int getFormattedMsg(unsigned char * data, int data_sz) = 0;
};


#endif /* __BASE_MSG__ */

