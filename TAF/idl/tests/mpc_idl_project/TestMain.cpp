/***************************************************************
    Copyright 2016, 2017 Defence Science and Technology Group,
    Department of Defence,
    Australian Government

	This file is part of LASAGNE.

    LASAGNE is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    LASAGNE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with LASAGNE.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************/


#include "TestTAFSupport.h"
#include "daf/Runnable.h"
#include "daf/Monitor.h"
#include "ace/Log_Msg.h"
#include "daf/TaskExecutor.h"
#include "taf/TAFServer.h"

const int num_of_messages = 10;

struct TopicReader : DAF::Monitor, DAF::Runnable, virtual dsto::DRAWDetails_Reader
{

  DDS::DomainId_t id;
  TAFDDS::DDS_DomainParticipant participant;

  volatile int count;

  TopicReader(const DDS::DomainId_t &id_in)
    : id(id_in)
    , count(0)
  {

    this->participant.init(id);

  }

  DDS::ReturnCode_t on_data_available(const dsto::DRAWDetails_TopicType &dt)
  {
    ACE_UNUSED_ARG(dt);
    ACE_DEBUG((LM_INFO, ACE_TEXT("RX DRAWDetails\n")));

    this->count++;
    this->notify();
    return DDS::RETCODE_OK;
  }


  int run(void)
  {
    TAFDDS::DDS_DomainParticipant participant;
    if ( participant.init(id) != DDS::RETCODE_OK )
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT(" ERROR: Unable to create participant\n")), -1);
    }

    dsto::DRAWDetails_Topic top;
    top.init(participant, "Test");

    dsto::DRAWDetails_Subscriber sub;
    sub.init(participant);

    if ( dsto::DRAWDetails_Reader::init(sub, top) != DDS::RETCODE_OK)
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT(" ERROR: Unable to create reader\n")), -1);
    }

    // main thread -sync
    this->notify();

    while ( this->count < num_of_messages) try
    {
      this->wait(1000);
    }
    catch (...)
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT(" ERROR: Unable to create participant\n")), -1);
    }


    return 0;
  }
};



int ACE_TMAIN (int argc, char *argv [])
{
  ACE_UNUSED_ARG(argc);
  ACE_UNUSED_ARG(argv);

  // Problems using DDS when there is no Server and ORB!
  TAFServer server(argc, argv); server.run(false);

  DDS::DomainId_t               domain_id(1);
  TAFDDS::DDS_DomainParticipant participant;

  if ( participant.init(domain_id) != DDS::RETCODE_OK )
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT(" ERROR: Unable to create participant\n")), -1);
  }

  TopicReader::_ref_type reader = new TopicReader(domain_id);

  DAF::SingletonExecute(reader);

  //reader->wait();

  dsto::DRAWDetails_Publisher pub;
  pub.init(participant);
  dsto::DRAWDetails_Topic top;
  top.init(participant, "Test");
  dsto::DRAWDetails_Writer writer;
  writer.init(pub, top);


  for (int i = 0 ;i < num_of_messages; ++i ) try
  {
    dsto::DRAWDetails_TopicType dt = { ::dsto::Triangle, i, CORBA::ULongLong(DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME)) }; writer << dt;
    DAF_OS::sleep(ACE_Time_Value(0, 100000));
  }
  catch(...)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT(" ERROR: Unable to publish \n")), -1);
  }

  // There is an issue on closeout here. and Loading.
  // The DDS infrastructure tries to install in the ORB/global gestalt, which we don't particularly want to use
  // TAFServer dying is changing the order of destruction and the Service_Config_Guards are problematically
  // accessing a null value

  return 0;
}
