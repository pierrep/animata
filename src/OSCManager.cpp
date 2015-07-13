/*
 Animata

 Copyright (C) 2007 Peter Nemeth, Gabor Papp, Bence Samu
 Kitchen Budapest, <http://animata.kibu.hu/>

 This file is part of Animata.

 Animata is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Animata is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Animata. If not, see <http://www.gnu.org/licenses/>.

*/

#include <iostream>
#include <unistd.h>

#include "animata.h"
#include "animataUI.h"

#include "OSCManager.h"

using namespace Animata;

OSCListener::OSCListener()
{
	thread = 0;
	rootLayer = NULL;
}

OSCListener::~OSCListener()
{
	stop();
}

void OSCListener::ProcessMessage(const osc::ReceivedMessage& m,
		const IpEndpointName& remoteEndpoint)
{


//	if (!rootLayer)
//	{
//	    cout << "OSCManager - Not rootLayer, ABORTING" << endl;
//		return;
//	}
//
//	if (!ui) // FIXME: this should not happen but it does
//    {
//         cout << "OSCManager - Not UI, ABORTING" << endl;
//		return;
//    }

	try
	{
		// TODO: write one function for each object type and call it
		// with a function pointer on each message
		if (strcmp(m.AddressPattern(), "/anibone") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			float val;
			args >> namePattern >> val >> osc::EndMessage;

			// FIXME: locking?, bones should not be deleted while this is
			// running
			lock();
			vector<Bone *> *bones = ui->editorBox->getAllBones();

			int found = 0;
			// try to find exact match for bone names first
			vector<Bone *>::iterator b = bones->begin();
			for (; b < bones->end(); b++)
			{
				const char *boneName = (*b)->getName();
				// skip unnamed bones
				if (boneName[0] == 0)
					continue;
				if (strcmp(boneName, namePattern) == 0)
				{
					(*b)->animateBone(val);
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Bone *>::iterator b = bones->begin();
				for (; b < bones->end(); b++)
				{
					const char *boneName = (*b)->getName();
					// skip unnamed bones
					if (boneName[0] == 0)
						continue;
					if (patternMatch(boneName, namePattern))
					{
						(*b)->animateBone(val);
					}
				}
			}

			unlock();
		}
		else if (strcmp(m.AddressPattern(), "/joint") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			float x, y;
			//args >> namePattern >> x >> y >> osc::EndMessage;
			osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
			namePattern = (arg++)->AsString();

			/* parameters can be float or int */
			if (arg->IsInt32())
				x = (arg++)->AsInt32();
			else
				x = (arg++)->AsFloat();
			if (arg->IsInt32())
				y = (arg++)->AsInt32();
			else
				y = (arg++)->AsFloat();
			if (arg != m.ArgumentsEnd()) {
                    cout << "Excess Argument Exception" << endl;
				throw osc::ExcessArgumentException();
			}

			lock();

			vector<Joint *> *joints = ui->editorBox->getAllJoints();

			int found = 0;
			// try to find exact match for joint names first
			vector<Joint *>::iterator j = joints->begin();
			for (; j < joints->end(); j++)
			{
				const char *jointName = (*j)->getName();
				// skip unnamed joints
				if (jointName[0] == 0)
					continue;
				if (strcmp(jointName, namePattern) == 0)
				{
					(*j)->x = x;
					(*j)->y = y;
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Joint *>::iterator j = joints->begin();
				for (; j < joints->end(); j++)
				{
					const char *jointName = (*j)->getName();
					// skip unnamed joints
					if (jointName[0] == 0)
						continue;
					if (patternMatch(jointName, namePattern))
					{
						(*j)->x = x;
						(*j)->y = y;
					}
				}
			}
			unlock();
		}
		else if (strcmp(m.AddressPattern(), "/layervis") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			bool val;

			osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
			namePattern = (arg++)->AsString();
			/* parameter can be boolean or int */
			if (arg->IsBool())
				val = (arg++)->AsBool();
			else
				val = (1 == (arg++)->AsInt32());
			if (arg != m.ArgumentsEnd())
				throw osc::ExcessArgumentException();

			// get all layers
			lock();
			vector<Layer *> *layers = ui->editorBox->getAllLayers();

			int found = 0;
			// try to find exact match for layer names first
			vector<Layer *>::iterator l = layers->begin();
			for (; l < layers->end(); l++)
			{
				const char *layerName = (*l)->getName();
				// skip unnamed layers
				if (layerName[0] == 0)
					continue;

				if (strcmp(layerName, namePattern) == 0)
				{
					(*l)->setVisibility(val);
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Layer *>::iterator l = layers->begin();
				for (; l < layers->end(); l++)
				{
					const char *layerName = (*l)->getName();
					// skip unnamed layers
					if (layerName[0] == 0)
						continue;
					if (patternMatch(layerName, namePattern))
					{
						(*l)->setVisibility(val);
						found = 1;
					}
				}
			}

			if (!found)
			{
				cerr << "OSC error: " << m.AddressPattern() << ": "
					<< "layer is not found" << "\n";
			}
			unlock();
		}
		else if (strcmp(m.AddressPattern(), "/layeralpha") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			float val;
			args >> namePattern >> val >> osc::EndMessage;

			// get all layers
			lock();
			vector<Layer *> *layers = ui->editorBox->getAllLayers();

			int found = 0;
			// try to find exact match for layer names first
			vector<Layer *>::iterator l = layers->begin();
			for (; l < layers->end(); l++)
			{
				const char *layerName = (*l)->getName();
				// skip unnamed layers
				if (layerName[0] == 0)
					continue;
				if (strcmp(layerName, namePattern) == 0)
				{
					(*l)->setAlpha(val);
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Layer *>::iterator l = layers->begin();
				for (; l < layers->end(); l++)
				{
					const char *layerName = (*l)->getName();
					// skip unnamed layers
					if (layerName[0] == 0)
						continue;
					if (patternMatch(layerName, namePattern))
					{
						(*l)->setAlpha(val);
						found = 1;
					}
				}
			}

			if (!found)
			{
				cerr << "OSC error: " << m.AddressPattern() << ": "
					<< "layer is not found" << "\n";
			}
			unlock();
		}
		else if (strcmp(m.AddressPattern(), "/layerpos") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			float x;
			float y;
			float z=0;
			//args >> namePattern >> x >> y >> osc::EndMessage;
			osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
			namePattern = (arg++)->AsString();
			/* parameters can be float or int */
			if (arg->IsInt32())
				x = (arg++)->AsInt32();
			else
				x = (arg++)->AsFloat();
			if (arg->IsInt32())
				y = (arg++)->AsInt32();
			else
				y = (arg++)->AsFloat();
			if (m.ArgumentCount() == 4)
			{
				if (arg->IsInt32())
					z = (arg++)->AsInt32();
				else
					z = (arg++)->AsFloat();
			}
			if (arg != m.ArgumentsEnd())
				throw osc::ExcessArgumentException();

			// get all layers
			lock();
			vector<Layer *> *layers = ui->editorBox->getAllLayers();

			int found = 0;
			// try to find exact match for layer names first
			vector<Layer *>::iterator l = layers->begin();
			for (; l < layers->end(); l++)
			{
				const char *layerName = (*l)->getName();
				// skip unnamed layers
				if (layerName[0] == 0)
					continue;
				if (strcmp(layerName, namePattern) == 0)
				{
					(*l)->setX(x);
					(*l)->setY(y);
					if (m.ArgumentCount() == 4)
						(*l)->setZ(z);
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Layer *>::iterator l = layers->begin();
				for (; l < layers->end(); l++)
				{
					const char *layerName = (*l)->getName();
					// skip unnamed layers
					if (layerName[0] == 0)
						continue;
					if (patternMatch(layerName, namePattern))
					{
						(*l)->setX(x);
						(*l)->setY(y);
						found = 1;
					}
				}
			}

			if (!found)
			{
				cerr << "OSC error: " << m.AddressPattern() << ": "
					<< "layer " << namePattern << " is not found" << "\n";
			}
			unlock();
		}
		else if (strcmp(m.AddressPattern(), "/layerdeltapos") == 0)
		{
			osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
			const char *namePattern;
			float x;
			float y;
			//args >> namePattern >> x >> y >> osc::EndMessage;
			osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
			namePattern = (arg++)->AsString();
			/* parameters can be float or int */
			if (arg->IsInt32())
				x = (arg++)->AsInt32();
			else
				x = (arg++)->AsFloat();
			if (arg->IsInt32())
				y = (arg++)->AsInt32();
			else
				y = (arg++)->AsFloat();
			if (arg != m.ArgumentsEnd())
				throw osc::ExcessArgumentException();

			// get all layers
			lock();
			vector<Layer *> *layers = ui->editorBox->getAllLayers();

			int found = 0;
			// try to find exact match for layer names first
			vector<Layer *>::iterator l = layers->begin();
			for (; l < layers->end(); l++)
			{
				const char *layerName = (*l)->getName();
				// skip unnamed layers
				if (layerName[0] == 0)
					continue;
				if (strcmp(layerName, namePattern) == 0)
				{
					(*l)->move(x, y);
					found = 1;
				}
			}

			// if exact match is not found try regular expression match
			if (!found)
			{
				vector<Layer *>::iterator l = layers->begin();
				for (; l < layers->end(); l++)
				{
					const char *layerName = (*l)->getName();
					// skip unnamed layers
					if (layerName[0] == 0)
						continue;
					if (patternMatch(layerName, namePattern))
					{
						(*l)->move(x, y);
						found = 1;
					}
				}
			}

			if (!found)
			{
				cerr << "OSC error: " << m.AddressPattern() << ": "
					<< "layer is not found" << "\n";
			}
			unlock();
		}
        else if (strcmp(m.AddressPattern(), "/updateTextures") == 0)
        {
            cout << "Updating Textures...";
            lock();
            ui->editorBox->flagUpdateTextures();
            unlock();
            cout << "Done!" << endl;
        }
	}
	catch (osc::Exception& e)
	{
		// any parsing errors get thrown as exceptions
		cerr << "OSC error: " << m.AddressPattern() << ": "
			<< e.what() << "\n";
	}
}

void *OSCListener::threadFunc(void *p)
{
	static_cast<OSCListener *>(p)->threadTask();
	return 0;
}

void OSCListener::start(void)
{
	if (thread == 0)
	{
		pthread_create(&thread, NULL, &threadFunc, this);
		pthread_setname_np(thread, "OSCListener");
		pthread_mutex_init(&mutex, NULL);
	}
}

void OSCListener::stop(void)
{
	if (thread)
	{
		// send a break to make the listener exit from its Run() state
		ulrs->AsynchronousBreak();
		pthread_join(thread, NULL);	// wait until the thread is complete
		pthread_mutex_destroy(&mutex);
		delete ulrs;
		thread = 0;
	}
}

void OSCListener::threadTask(void)
{
    ulrs = new UdpListeningReceiveSocket(
			IpEndpointName(IpEndpointName::ANY_ADDRESS, OSC_RECEIVE_PORT),
            this);

	// run until a break is sent from the other thread
    ulrs->Run();
}


void OSCListener::lock(void)
{
	pthread_mutex_lock(&mutex);
}

void OSCListener::unlock(void)
{
	pthread_mutex_unlock(&mutex);
}

/* FIXME: is this needed? */
void OSCListener::setRootLayer(Layer *root)
{
	lock();
	rootLayer = root;
	unlock();
}

/*
 * robust glob pattern matcher
 * by ozan s. yigit, modified by daniel holth
 *
 * ozan s. yigit/dec 1994
 * public domain
 *
 * glob patterns:
 *  *   matches zero or more characters
 *  ?   matches any single character
 *  [set]   matches any character in the set
 *  [^set]  matches any character NOT in the set
 *      where a set is a group of characters or ranges. a range
 *      is written as two characters separated with a hyphen: a-z denotes
 *      all characters between a to z inclusive.
 *  [-set]  set matches a literal hypen and any character in the set
 *  []set]  matches a literal close bracket and any character in the set
 *
 *  char    matches itself except where char is '*' or '?' or '['
 *  \char   matches char, including any pattern character
 *
 * examples:
 *  a*c     ac abc abbc ...
 *  a?c     acc abc aXc ...
 *  a[a-z]c     aac abc acc ...
 *  a[-a-z]c    a-c aac abc ...
 *
 * $Log$
 * Revision 1.1  2004/11/19 23:00:57  theno23
 * Added lo_send_timestamped
 *
 * Revision 1.3  1995/09/14  23:24:23  oz
 * removed boring test/main code.
 *
 * Revision 1.2  94/12/11  10:38:15  oz
 * cset code fixed. it is now robust and interprets all
 * variations of cset [i think] correctly, including [z-a] etc.
 *
 * Revision 1.1  94/12/08  12:45:23  oz
 * Initial revision
 */

#ifndef NEGATE
#define NEGATE  '!'
#endif

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/**
 * Matches string to regex pattern.
 * \param str string to match
 * \param p pattern
 * \return 1 on match
 **/
int OSCListener::patternMatch(const char *str, const char *p)
{
	int negate;
	int match;
	char c;

	while (*p)
	{
		if (!*str && *p != '*')
			return false;

		switch (c = *p++)
		{
			case '*':
				while (*p == '*')
					p++;

				if (!*p)
					return true;

				// if (*p != '?' && *p != '[' && *p != '\\')
				if (*p != '?' && *p != '[' && *p != '{')
					while (*str && *p != *str)
						str++;

				while (*str)
				{
					if (patternMatch(str, p))
						return true;
					str++;
				}
				return false;

			case '?':
				if (*str)
					break;
				return false;
			/*
			 * set specification is inclusive, that is [a-z] is a, z and
			 * everything in between. this means [z-a] may be interpreted
			 * as a set that contains z, a and nothing in between.
			 */
			case '[':
				if (*p != NEGATE)
					negate = false;
				else
				{
					negate = true;
					p++;
				}

				match = false;

				while (!match && (c = *p++))
				{
					if (!*p)
						return false;
					if (*p == '-')
					{    /* c-c */
						if (!*++p)
							return false;
						if (*p != ']')
						{
							if (*str == c || *str == *p ||
									(*str > c && *str < *p))
								match = true;
						}
						else
						{      /* c-] */
							if (*str >= c)
								match = true;
							break;
						}
					}
					else
					{          /* cc or c] */
						if (c == *str)
							match = true;
						if (*p != ']')
						{
							if (*p == *str)
								match = true;
						}
						else
							break;
					}
				}

				if (negate == match)
					return false;
				/*
				 * if there is a match, skip past the cset and continue on
				 */
				while (*p && *p != ']')
					p++;
				if (!*p++)  /* oops! */
					return false;
				break;

			/*
			 * {astring,bstring,cstring}
			 */
			case '{':
				{
					// *p is now first character in the {brace list}
					const char *place = str;    // to backtrack
					const char *remainder = p;  // to forwardtrack

					// find the end of the brace list
					while (*remainder && *remainder != '}')
						remainder++;
					if (!*remainder++)  /* oops! */
						return false;

					c = *p++;

					while (*p)
					{
						if (c == ',')
						{
							if (patternMatch(str, remainder))
							{
								return true;
							}
							else
							{
								// backtrack on test string
								str = place;
								// continue testing,
								// skip comma
								if (!*p++)  // oops
									return false;
							}
						}
						else if (c == '}')
						{
							// continue normal pattern matching
							if(!*p++)
								return false;
							break;
						}
						else if (c == *str)
						{
							str++;
							if (!*str && *remainder)
								return false;
							// p++;
						}
						else
						{ // skip to next comma
							str = place;
							while (*p != ',' && *p != '}' && *p)
								p++;
							if (*p == ',')
								p++;
						}
						c = *p++;
					}
				}

				break;

				/* Not part of OSC pattern matching
				   case '\\':
				   if (*p)
				   c = *p++;
				   */

			default:
				if (c != *str)
					return false;
				break;

		}
		str++;
	}

	return !*str;
}


OSCSender::OSCSender(const char *hostName)
{
	thread = 0;
	IpEndpointName host(hostName, OSC_SEND_PORT);

	ops = new osc::OutboundPacketStream(buffer, IP_MTU_SIZE);
	socket = new UdpTransmitSocket(host);
}

OSCSender::~OSCSender()
{
	stop();
}

void *OSCSender::threadFunc(void *p)
{
	static_cast<OSCSender *>(p)->threadTask();
	return 0;
}

void OSCSender::start(void)
{
	if (thread == 0)
	{
		threadRunning = true;
		pthread_create(&thread, NULL, &threadFunc, this);
		pthread_mutex_init(&mutex, NULL);
	}
}

void OSCSender::stop(void)
{
	if (thread)
	{
		// send a break to make the sender exit
		threadRunning = false;
		pthread_join(thread, NULL);	// wait until the thread is complete
		pthread_mutex_destroy(&mutex);
		thread = 0;
	}
}

void OSCSender::threadTask(void)
{
	while (threadRunning && (ui != NULL))
	{
		vector<Joint *> *oscJoints = ui->editorBox->getOSCJoints();
		if (oscJoints != NULL)
		{
			vector<Joint *>::iterator ji = oscJoints->begin();
			for (; ji < oscJoints->end(); ji++)
			{
				Joint *j = *ji;
				ops->Clear();
				(*ops) << osc::BeginBundleImmediate <<
					osc::BeginMessage("/joint") <<
					j->getName() <<
					j->x << j->y << osc::EndMessage <<
					osc::EndBundle;
				socket->Send(ops->Data(), ops->Size());
			}
		}

		// send messages 25 times per second approximately
		usleep(40000);
	}
}


void OSCSender::lock(void)
{
	pthread_mutex_lock(&mutex);
}

void OSCSender::unlock(void)
{
	pthread_mutex_unlock(&mutex);
}

