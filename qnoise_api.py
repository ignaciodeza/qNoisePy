from flask import Flask, request
from flask_cors import CORS
from qNoisePy import qNoise
import os

tokenList = ['a701165f12b9e3e2ebfef521fc2835b111b2fc21','1f686d8d973c21affc9b37011978c582f7885881']

app = Flask(__name__)
cors = CORS(app)

@app.route('/', methods=['OPTIONS','GET'])
def test_api():
	tau = 0.01
	q = 1.1
	N = 10
	A = qNoise(tau=tau,q=q,N=N)
	R = {'success' : 'true','tau':tau,'q':q,'N':N,'data':list(A)}
	msg=''
	msg+='<h1>qNoise API is working!</h1>'
	msg+='<h2>Please use via POST, send JSON file with tau, q, and N. See documentation.</h2>'
	msg+='<h3>Here are 10 random numbers, as a treat:</h3>'
	msg+="<br>".join(["{0:2f}".format(x) for x in R['data']])
	return msg

@app.route('/', methods=['OPTIONS','POST'])
def qNoise_api():
	'''Generate non-Gaussian Coloured data with the data provided in the JSON file'''
	JSON = request.get_json()
	if JSON is not None and JSON['apikey'] not in  tokenList:
		return {'success' : 'false', 'error':'invalid token'}

	params = {}
	try:
		params['q'] = JSON['q']
		params['tau'] = JSON['tau']
		params['N'] = JSON['N']
	except:
		return {'success' : 'false', 'error':'invalid JSON file'}
	try:
		params['H'] = JSON['H']
	except:
		pass
	try:
		params['norm'] = JSON['norm']
	except:
		pass
	try:
		useDict = JSON['usedict'] == 'true'
	except:
		useDict = False


	A = qNoise(**params)
	if useDict:
		R = {'success' : 'true','tau':params['tau'],'q':params['q'],'N':params['N'],'data':[{'index':i,'value':x} for i,x in enumerate(list(A))]}
	else:
		R = {'success' : 'true','tau':params['tau'],'q':params['q'],'N':params['N'],'data':list(A)}
	return R

if __name__ == "__main__":
	# Threaded option to enable multiple instances for multiple user access support
	#app.run(threaded=True, port=5111,host='0.0.0.0')
	#app.run(debug=True,port=5111,host='0.0.0.0')
	app.run(host="0.0.0.0", port=int(os.environ.get("PORT", 5000)))

