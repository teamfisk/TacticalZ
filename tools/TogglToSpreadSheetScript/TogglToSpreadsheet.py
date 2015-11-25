import base64
import sys
import re
import pip
try:
    import requests
except ImportError:
    print 'requests module not installed.'
    print 'Installing requests...'
    pip.main(['install', 'requests'])
    import requests

try:
    import gspread
except ImportError:
    print 'gspread module not installed.'
    print 'Installing gspread...'
    pip.main(['install', 'gspread'])
    import gspread

import json
import time
from oauth2client.client import SignedJwtAssertionCredentials

def write_to_sheet(values, estimatedTime):
    # Note: This is Andreas client email & private google API key. Plz no share
    client_email = 'account-1@toggltospreadsheet.iam.gserviceaccount.com'
    private_key = 'nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQCvgdm4HeS54TS9\nS58jhxbaxrFL5yOyyofJo51CtBAMT0aCuV41ngoethfJArjwO1ZziaPRX0ZrQvBC\n3XtIcBblkR/6w0uYF3lTcwuxqpbNrkptQNc7ms91pYHeRJS/u+U0hmnmS+TI8q4c\nm47xbtpw2ym8Uvq1anvYgmnlQQNNgaFcuYSgFLPhRL2TdOR82fz055rHUeEIT3YG\n7RpZBokWQFaZT5KsWbBjjEMlfexg11/9tK1zbDCjqU/JSBRmzmXh0kEbRCWQWC5k\n+ZHSrcD8Sj3d6nfBsxiu8yAW0pYqMEDNnlX69ovqjVlfa1PO+J+7IwY1svde872C\nzrC1huyzAgMBAAECggEASiq2he7kDIUWE3SUkJ/y0Ysru2a3GEQsM9LXjyumqH0L\n0AxjuobJwgazcHedDbAVrYeZ2c3IZWWJQMh147uygVrdx8ul82TgGZrBc1gimFKy\nEw9WpVKbnxzND8+tiITvrE2tDOw/h4e+ekpmkrKEzzJepb3vQqD4KxuZgo8BxUtw\n1bBAq2hKKukc1IlzXv0cegWYqfw1wA3vFngQh+B/ueR3kbLUWu130z2Ls/SnBdHt\nYaIbER6YUrQ6eG2cQrjh711zMjMgMsinyh+MR/VHERspPR7HnNJkEb8LXivoLQE4\nUbBTihups769NJUr3CSqKXHIwv8QO9EfJ6IN8pglwQKBgQDyMEJW3iE0tKaBgtXY\nzJ4TWOkTOoKAnbuZd19ZbqARPfa+6apT4NWCz7pVXcZKv7rsMZTbCHAf3HfptCAq\nQSVkuRZaCv6SOUaWp0kk8cxIJpbvwMeqBY/BDmQr8SzUir6N0M+7G3tV/joqpY5c\n2V1udZeR0Ch4V+F9M/osmyvOIQKBgQC5hBpQlMmeQivk9P1eHkiB1w9RlLIrYjcQ\ndaSFOwgMiswK2/I04P/9BhXXeIin5r5EmpfHOM32wTdu5RkU5Ts428JqAshQBSwZ\nEciofEJ0fvmxCR+d+imG+Kq2BOneatOC6aYBVWr2VUDq8KyD4jL8tju6KOLvssRR\nsoK26wAYUwKBgQCexHpI3jfgiGj7UB0GkiUyw7+P5nR1AnJgSfxM8ZOnmfpu71nE\nwQjXR3x8yAvdJtHQUzSlXmO6z1og7/+CE9ECtb9safa3PysCSkpOGOF1jy61n6iE\n0j6KLfgHQoTEFOyUpYX4wCxblFznZj7sqWZxqk8hvNc7BUmCPZfMtDDEYQKBgQCP\nK+ZrHgjjvEnH71LCmjh3DBRkb495b9jzOPd5Yu95TnzePJSWPrcQ/OtKWVmNysQ4\nid5s/+fkcYVobiKHP8oOvXsy+WbCatt3lYP4k71tzrjA6jueXfxCkBKfWvdqkaMe\nu1dEXDmqVm09Y/Sf66hR5AoAR6GsP5jHPC8pH//4xQKBgQDlVw8Q6Xq5FFUfURvF\n6p8XsCgheykLDzFZHv2neL4ESXpt2flCaLcb+pC8+IrNDorC8BX6JiZD2WngMn+s\nClAkq4TgGE5Mjmk5ZOhrpg69H7O87OcNK5Cc12+tnoPsnvEcuF8CoGWIy3A6PNt+\nROnL6yOGeh2IHowPjrO3ODKO4Q=='
    
    # Change the path, otherwise the the json_key will not be found and writing to the Sheet will fail.
    json_key = json.load(open('c:/python27/TogglToSpreadSheet-cca18cfac737.json'))
    scope = ['https://spreadsheets.google.com/feeds']

    credentials = SignedJwtAssertionCredentials(json_key['client_email'], json_key['private_key'].encode(), scope)
    gc = gspread.authorize(credentials)
    sh = gc.open_by_url('https://docs.google.com/spreadsheets/d/1P8HFfktwSAF0rgi9rxpNvdMdR0GYuo845qAGikeJUh8/edit#gid=0')
    worksheet = sh.get_worksheet(0)

    currentDate = time.strftime("%Y-%m-%d")
    results = [currentDate,values]
    worksheet.insert_row(results, 3)
    worksheet.update_acell("F2", estimatedTime)
    return;

def get_totaltime_data():
    api_token = '136ab033e06f9202497093b989f59f39'
    _workspace_id = 1190663
    print 'Sending Request...'
    
    r = requests.get('https://toggl.com/reports/api/v2/summary', auth=(api_token, 'api_token'), params={'workspace_id': _workspace_id, 'since' : '2015-11-01', 'user_agent': 'api_test'})
    if r.status_code != 200:
        print 'Request Failed. Check your API Token'
        return;
    index = []

    rWorkspace = requests.get('https://www.toggl.com/api/v8/workspaces/1190663/projects', auth=(api_token, 'api_token'))
    workspaceText = rWorkspace.text
    taskMap = map(int, re.findall(r'\d+', workspaceText))
    estimatedTime = 0
    counter = 0
    index = [x-1 for x, i in enumerate(taskMap) if i == 1190663]
    

    for x in index:
        timeMap = []
        rProject = requests.get('https://www.toggl.com/reports/api/v2/project', auth=(api_token, 'api_token'), params={'user_agent': 'api_test','workspace_id': _workspace_id, 'project_id': taskMap[x]})
        if r.status_code != 200:
            print 'rProject request Failed. Check your API Token'
            return;
        projectText = rProject.text
        text = projectText.replace('null','0')
        finalText = text.replace('0.0', '0')
        timeMap = map(int, re.findall(r'\d+', finalText))
        estimatedTime = estimatedTime + timeMap[6]
        counter+= 1

    wholeText = r.text
    allNumbers = map(int, re.findall('\d+', wholeText))
    totalTime = allNumbers[0]
    timeleft = totalTime
    hours = totalTime / 3600000
    timeleft -=  hours * 3600000
    min = timeleft / 60000
    timeleft -= min * 60000 
    sec = timeleft / 1000
    estimatedTimeInHours = estimatedTime / 3600
    
    str = 'Total Time: ' + repr(hours) +':' + repr(min) + ':' + repr(sec)
    print str
    result = repr(hours) + ':' + repr(min) +':' + repr(sec)
    #result = []
    #result.append(hours);
    #result.append(min);
    #result.append(sec);
    
    print 'Writing to Sheet...'
    write_to_sheet(result, estimatedTimeInHours)
    return;

get_totaltime_data()
print 'Done!'
