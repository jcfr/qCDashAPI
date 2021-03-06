/*==============================================================================

  Library: qRestAPI

  Copyright (c) 2013 University College London, Centre for Medical Image Computing

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Original author: Miklos Espak <m.espak@ucl.ac.uk>

==============================================================================*/

// qXnatAPI includes
#include "qXnatAPI.h"
#include "qXnatAPI_p.h"
#include "qRestResult.h"
#include <QNetworkReply>
#include <QRegExp>
#include <QUrl>

// --------------------------------------------------------------------------
// qXnatAPIPrivate methods

// --------------------------------------------------------------------------
qXnatAPIPrivate::qXnatAPIPrivate(qXnatAPI* object)
  : Superclass(object)
  , q_ptr(object)
{
}

// --------------------------------------------------------------------------
QList<QVariantMap> qXnatAPIPrivate::parseXmlResponse(qRestResult* restResult, const QByteArray& response)
{
  QList<QVariantMap> result;
  return result;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qXnatAPIPrivate::parseJsonResponse(qRestResult* restResult, const QByteArray& response)
{
  Q_Q(qXnatAPI);

  QScriptValue scriptValue = this->ScriptEngine.evaluate("(" + QString(response) + ")");

  QList<QVariantMap> result;

  // e.g. {"ResultSet":{"Result": [{"p1":"v1","p2":"v2",...}], "totalRecords":"13"}}
  QScriptValue resultSet = scriptValue.property("ResultSet");
  QScriptValue dataLength = resultSet.property("totalRecords");
  QScriptValue data = resultSet.property("Result");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      restResult->setError("No data");
      }
    else
      {
      restResult->setError(QString("Bad data: ") + data.toString());
      }
    }
  if (data.isArray())
    {
    quint32 length = data.property("length").toUInt32();
    for(quint32 i = 0; i < length; ++i)
      {
      qRestAPI::appendScriptValueToVariantMapList(result, data.property(i));
      }
    }
  else
    {
    qRestAPI::appendScriptValueToVariantMapList(result, data);
    }

  return result;
}

// --------------------------------------------------------------------------
// qXnatAPI methods

// --------------------------------------------------------------------------
qXnatAPI::qXnatAPI(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qXnatAPIPrivate(this))
{
}

// --------------------------------------------------------------------------
qXnatAPI::~qXnatAPI()
{
}

// --------------------------------------------------------------------------
QUuid qXnatAPI::get(const QString& resource, const Parameters& parameters, const qRestAPI::RawHeaders& rawHeaders)
{
  Q_D(qXnatAPI);
  QUrl url = this->createUrl(resource, parameters);
  url.addQueryItem("format", "json");
  QNetworkReply* queryReply = this->sendRequest(QNetworkAccessManager::GetOperation, url, rawHeaders);
  QUuid queryId = queryReply->property("uuid").toString();
  return queryId;
}

// --------------------------------------------------------------------------
void qXnatAPI::parseResponse(qRestResult* restResult, const QByteArray& response)
{
  Q_D(qXnatAPI);

  static QRegExp identifierPattern("[a-zA-Z][a-zA-Z0-9_]*");

  QList<QVariantMap> result;

  if (response.isEmpty())
    {
    // Some operations do not return result. E.g. creating a project.
    }
  else if (response.startsWith("<html>"))
    {
    // Some operations return an XML description of an object.
    // E.g. GET query for a specific subject.
    restResult->setError(QString("Bad data: ") + response);
    }
  else if (response.startsWith("<?xml "))
    {
    // Some operations return an XML description of an object.
    // E.g. GET query for a specific subject.
    result = d->parseXmlResponse(restResult, response);
    }
  else if (response[0] == '{')
    {
    // Other operations return a json description of an object.
    // E.g. GET query of the list of subjects
    result = d->parseJsonResponse(restResult, response);
    }
  else if (identifierPattern.exactMatch(response))
    {
    // Some operations return the identifier of the newly created object.
    // E.g. creating a subject.
    QVariantMap map;
    map["ID"] = response;
    result.push_back(map);
    }
  else
    {
    restResult->setError(QString("Bad data: ") + response);
    }

  restResult->setResult(result);
}
