How to start the chatbot:
1. start ngrok: ./ngrok http 5005
2. create webex webhook (https://developer.webex.com/docs/api/v1/webhooks/create-a-webhook) with targetUrl: url returned by ngrok + /webhooks/webexteams/webhook suffix
3. start rasa actions: rasa run actions
4. start rasa: rasa run --enable-api
5. find "Chwile Przyjemnosci Restaurant" chatbot on Webex