#!/usr/bin/env python
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
import re

queue = []
invalid = '[!@#$%\^&*()+=-\[\]{}\"\'?/><;:.,]'
class Gomoku(LineReceiver):
#
	def __init__(self, users):
		self.users = users
		self.name = None
		self.opponent = None
		self.blacklist = []
		self.status = 'Getname'

	def connectionMade(self):
		print 'Connection:', self.transport.client[0]
		self.sendLine('What\'s your name?')

	def connectionLost(self, reason):
		if self.users.has_key(self.name):
			print 'Disconnection:', self.transport.client[0]

	def lineReceived(self, line):
		if line == '':
			return
		print self.name, self.status,'(%s) send:' %(self.transport.client[0]), line
		if self.status == 'Getname':
			self.HANDLE_NAME(line)
		else:
			self.HANDLE_CMD(line)

	def BroadCast(self, string):
		for name, user in self.users.iteritems():
			user.sendLine('Server: '+string)

	def HANDLE_NAME(self, name):
		if len(re.findall(invalid, name)) != 0:
			self.sendLine('invalid')
			return
		if self.users.has_key(name):
			self.sendLine('Name taken, please choose another.')
			return
		self.sendLine('Welcome, %s!' % (name,))
		self.name = name
		self.users[name] = self
		self.blacklist = []
		self.status = 'Named'
		self.BroadCast('\''+name+'\' just login!')

	def HANDLE_CMD(self, str):
		info={
				'cmd'	: None,
				'name'	: None,
				'msg'	: None,
				'error'	: None,
				'found'	: False,
			 }

		info['cmd'] = str.split(' ')[0].replace('/', '').lower()
		for key, list in self.CmdMap.iteritems():
			if info['cmd'] in list:
				info['found'] = True
				info = self.CmdList[info['cmd']][1](self, str, info)
		if not info['found']:
			self.sendLine('Command not found.')
			return
		elif info['error'] != None:
			self.sendLine(info['error'])
			return

		self.CmdList[info['cmd']][2](self, info)
#
	def BB (self, str, info):
		str = str.split()[1:]
		if len(str) != 0:
			info ['error'] = 'Usage: '+ self.CmdList[info['cmd']][0]
		return info

	def AB (self, str, info):
		str = str.split()[1:]
		if len(str) != 1:
			info ['error'] = 'Usage: '+ self.CmdList[info['cmd']][0]
			return info
		else:
			info ['name'] = str[0]
		return info

	def BA (self, str, info):
		str = str.split()[1:]
		info['msg'] = ' '.join(str)
		return info

	def AA (self, str, info):
		str = str.split()[1:]
		if len(str) <= 1:
			info ['error'] = 'Usage: '+ self.CmdList[info['cmd']][0]
			return info
		info['name'] = str[0]
		info['msg'] = ' '.join(str[1:])
		return info
#
	def ListCmd (self, info):
		self.sendLine('<CommnadList-Start>')
		for item in [(k, self.CmdList[k]) for k in sorted(self.CmdList.keys())]:
			self.sendLine(item[1][0])
		self.sendLine('<End>')

	def ListUser (self, info):
		self.sendLine('<UserList-Start>')
		for item in [(k, self.users[k].transport.client[0]) for k in sorted(self.users.keys())]:
			self.sendLine(item[0]+' ('+item[1]+')')
		self.sendLine('<End>')

	def Rename (self, info):
		if len(re.findall(invalid, info['name'])) != 0:
			self.sendLine('invalid')
			return
		if self.users.has_key(info['name']):
			self.sendLine('Name taken, please choose another.')
			return
		self.BroadCast('User '+self.name+' was rename ('+info['name']+')')
		del self.users[self.name]
		self.name = info['name']
		self.users[self.name] = self
		self.sendLine('You are '+self.name+' now.')

	def Whisper (self, info):
		if not self.users.has_key(info['name']):
			self.sendLine('User not found.')
			return
		self.users[info['name']].sendLine(self.name+'(whisper): '+info['msg'])

	def Chat (self, info):
		for name, user in self.users.iteritems():
			if user == self or user in self.blacklist:
				pass
			else:
				user.sendLine(self.name+': '+info['msg'])

	def Ban (self, info):
		if not self.users.has_key(info['name']):
			self.sendLine('User not found.')
			return
		if info['name'] == self.name:
			self.sendLine('You can\'t ban yourself.')
			return
		self.blacklist.append(self.users[info['name']])
		self.sendLine('You\'ve banned '+ info['name'])

	def Unban (self, info):
		if not self.users.has_key(info['name']):
			self.sendLine('User not found.')
			return
		if self.users[info['name']] not in self.blacklist:
			self.sendLine('You never ban '+ info['name'])
			return
		self.blacklist.remove(self.users[info['name']])
		self.sendLine('You\'ve unbanned '+ info['name'])

	def Blacklist (self, info):
		self.sendLine('<BlackList-Start>')
		list = []
		for item in self.blacklist:
			list.append(item.name)
		for item in sorted(list):
			self.sendLine(item)
		self.sendLine('<End>')
#
	CmdList={
			'cmd'		: ['/cmd - List all ommands.'							, BB, ListCmd],
			'list'		: ['/list - List all users.'							, BB, ListUser],
			'name'		: ['/name [newname] - Rename your nickname.'			, AB, Rename],
			'w'			: ['/w [username] [messgage] - Send whisper to an user.', AA, Whisper],
			'all'		: ['/all [messgage] - Send messgage to all users.'		, BA, Chat],
			'ban'		: ['/ban [username] - Ban an user.'						, AB, Ban],
			'unban'		: ['/unban [username] - Unban an user.'					, AB, Unban],
			'blacklist'	: ['/blacklist - List your blacklist.'					, BB, Blacklist]
			}

	CmdMap={
			0: ['cmd', 'list', 'blacklist'], #/cmd
			1: ['name', 'ban', 'unban'],	 #/cmd name
			2: ['all', 'gomoku'],			 #/cmd msg
			3: ['w']						 #/cmd name msg
		   }
#
class GomokuFactory(Factory):

	def __init__ (self):
		self.users={}

	def buildProtocol(self, addr):
		return Gomoku(self.users)

port = 6666
IP = ''
reactor.listenTCP(port, GomokuFactory(), 10, IP)
reactor.run()
