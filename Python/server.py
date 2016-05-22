#!/usr/bin/env python
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor

queue = []

class Gomoku(LineReceiver):

	def __init__(self, users):
		self.users = users
		self.name = None
		self.status = 'Getname'

	def connectionMade(self):
		print 'Connection:', self.transport.client[0]
		self.sendLine('What\'s your name?')

	def connectionLost(self, reason):
		if self.users.has_key(self.name):
			print 'Disconnection:', self.transport.client[0], reason
			del self.users[self.name]

	def lineReceived(self, line):
		print self.name, '(%s)' %(self.transport.client[0]), line
		if self.status == 'Getname':
			self.HANDLE_NAME(line)
		else:
			self.HANDLE_GOMOKU(line)

	def HANDLE_NAME(self, name):
		if self.users.has_key(name):
			self.sendLine('Name taken, please choose another.')
			return
		self.sendLine('Welcome, %s!' % (name,))
		self.name = name
		self.users[name] = self
		self.status = 'Wait'
		queue.append(self)
		if len(queue) == 2:
			queue[0].enemy = queue[1]
			queue[1].enemy = queue[0]
			queue[0].sendLine('Your enemy is: %s' %(queue[1].name))
			queue[1].sendLine('Your enemy is: %s' %(queue[0].name))
			queue[0].status = queue[1].status = 'Gomoku'
			del queue[:]

	def HANDLE_GOMOKU(self, str):
		if str == 'ls':
			for name, protocol in self.users.iteritems():
				if protocol == self:
					s = name+' ('+self.transport.client[0]+')'
					self.sendLine(s)
				else:
					self.sendLine(name)
			return
		msg = str.split(' ')
		if msg[0] == '/g':
			self.enemy.sendLine(msg[1])
		elif msg[0] == '/c':
			for name, protocol in self.users.iteritems():
				if protocol != self:
					protocol.sendLine(msg[1])

class GomokuFactory(Factory):

	def __init__ (self):
		self.users={}

	def buildProtocol(self, addr):
		return Gomoku(self.users)

port = 33333
IP = '192.168.1.105'
reactor.listenTCP(port, GomokuFactory(), 10, IP)
reactor.run()
