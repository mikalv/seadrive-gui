extern "C" {

#include <searpc-client.h>
#include <searpc-named-pipe-transport.h>

}

#include <QtDebug>
#include <QMutexLocker>

#include "seadrive-gui.h"
#include "settings-mgr.h"

#include "utils/utils.h"
#include "api/commit-details.h"
#include "rpc-client.h"


namespace {

const char *kSeadriveSockName = "seadrive.sock";
const char *kSeadriveRpcService = "seadrive-rpcserver";

} // namespace

SeafileRpcClient::SeafileRpcClient()
      : seadrive_rpc_client_(0)
{
}

SeafileRpcClient::~SeafileRpcClient()
{
    if (seadrive_rpc_client_) {
        searpc_free_client_with_pipe_transport(seadrive_rpc_client_);
        seadrive_rpc_client_ = 0;
    }
}

void SeafileRpcClient::connectDaemon()
{
    SearpcNamedPipeClient *pipe_client = searpc_create_named_pipe_client(
        toCStr(QDir(gui->seadriveDataDir()).filePath(kSeadriveSockName)));
    if (searpc_named_pipe_client_connect(pipe_client) < 0) {
        gui->errorAndExit(tr("failed to connect to seadrive daemon"));
        return;
    }
    seadrive_rpc_client_ = searpc_client_with_named_pipe_transport(
        pipe_client, kSeadriveRpcService);
}

int SeafileRpcClient::seafileGetConfig(const QString &key, QString *value)
{
    GError *error = NULL;
    char *ret = searpc_client_call__string (seadrive_rpc_client_,
                                            "seafile_get_config", &error,
                                            1, "string", toCStr(key));
    if (error) {
        qWarning("Unable to get config value %s: %s", key.toUtf8().data(), error->message);
        g_error_free(error);
        return -1;
    }
    *value = QString::fromUtf8(ret);

    g_free (ret);
    return 0;
}

int SeafileRpcClient::seafileGetConfigInt(const QString &key, int *value)
{
    GError *error = NULL;
    *value = searpc_client_call__int (seadrive_rpc_client_,
                                      "seafile_get_config_int", &error,
                                      1, "string", toCStr(key));
    if (error) {
        qWarning("Unable to get config value %s: %s", key.toUtf8().data(), error->message);
        g_error_free(error);
        return -1;
    }
    return 0;
}


int SeafileRpcClient::seafileSetConfig(const QString &key, const QString &value)
{
    // printf ("set config: %s = %s\n", toCStr(key), toCStr(value));
    GError *error = NULL;
    searpc_client_call__int (seadrive_rpc_client_,
                             "seafile_set_config", &error,
                             2, "string", toCStr(key),
                             "string", toCStr(value));
    if (error) {
        qWarning("Unable to set config value %s", key.toUtf8().data());
        g_error_free(error);
        return -1;
    }
    return 0;
}

int SeafileRpcClient::setUploadRateLimit(int limit)
{
    return setRateLimit(true, limit);
}

int SeafileRpcClient::setDownloadRateLimit(int limit)
{
    return setRateLimit(false, limit);
}

int SeafileRpcClient::setRateLimit(bool upload, int limit)
{
    GError *error = NULL;
    const char *rpc = upload ? "seafile_set_upload_rate_limit" : "seafile_set_download_rate_limit";
    searpc_client_call__int (seadrive_rpc_client_,
                             rpc, &error,
                             1, "int", limit);
    if (error) {
        g_error_free(error);
        return -1;
    }
    return 0;
}

int SeafileRpcClient::seafileSetConfigInt(const QString &key, int value)
{
    // printf ("set config: %s = %d\n", toCStr(key), value);
    GError *error = NULL;
    searpc_client_call__int (seadrive_rpc_client_,
                             "seafile_set_config_int", &error,
                             2, "string", toCStr(key),
                             "int", value);
    if (error) {
        g_error_free(error);
        return -1;
    }
    return 0;
}

int SeafileRpcClient::getDownloadRate(int *rate)
{
    GError *error = NULL;
    int ret = searpc_client_call__int (seadrive_rpc_client_,
                                       "seafile_get_download_rate",
                                       &error, 0);

    if (error) {
        g_error_free(error);
        return -1;
    }

    *rate = ret;
    return 0;
}

int SeafileRpcClient::getUploadRate(int *rate)
{
    GError *error = NULL;
    int ret = searpc_client_call__int (seadrive_rpc_client_,
                                       "seafile_get_upload_rate",
                                       &error, 0);

    if (error) {
        g_error_free(error);
        return -1;
    }

    *rate = ret;
    return 0;
}


int SeafileRpcClient::getRepoProperty(const QString &repo_id,
                                      const QString& name,
                                      QString *value)
{
    GError *error = NULL;
    char *ret = searpc_client_call__string (
        seadrive_rpc_client_,
        "seafile_get_repo_property",
        &error, 2,
        "string", toCStr(repo_id),
        "string", toCStr(name)
        );
    if (error) {
        g_error_free(error);
        return -1;
    }
    *value = QString::fromUtf8(ret);

    g_free(ret);
    return 0;
}

int SeafileRpcClient::setRepoProperty(const QString &repo_id,
                                      const QString& name,
                                      const QString& value)
{
    GError *error = NULL;
    int ret = searpc_client_call__int (
        seadrive_rpc_client_,
        "seafile_set_repo_property",
        &error, 3,
        "string", toCStr(repo_id),
        "string", toCStr(name),
        "string", toCStr(value)
        );
    if (error) {
        g_error_free(error);
        return -1;
    }
    return ret;
}


int SeafileRpcClient::setRepoToken(const QString &repo_id,
                                   const QString& token)
{
    GError *error = NULL;
    int ret = searpc_client_call__int (
        seadrive_rpc_client_,
        "seafile_set_repo_token",
        &error, 2,
        "string", toCStr(repo_id),
        "string", toCStr(token)
        );
    if (error) {
        g_error_free(error);
        return -1;
    }
    return ret;
}

int SeafileRpcClient::getRepoFileStatus(const QString& repo_id,
                                        const QString& path_in_repo,
                                        bool isdir,
                                        QString *status)
{
    GError *error = NULL;
    char *ret = searpc_client_call__string (
        seadrive_rpc_client_,
        "seafile_get_path_sync_status",
        &error, 3,
        "string", toCStr(repo_id),
        "string", toCStr(path_in_repo),
        "int", isdir ? 1 : 0);
    if (error) {
        qWarning("failed to get path status: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    *status = ret;

    g_free (ret);
    return 0;
}

int SeafileRpcClient::markFileLockState(const QString &repo_id,
                                        const QString &path_in_repo,
                                        bool lock)
{
    GError *error = NULL;
    char *ret = searpc_client_call__string (
        seadrive_rpc_client_,
        lock ? "seafile_mark_file_locked" : "seafile_mark_file_unlocked",
        &error, 2,
        "string", toCStr(repo_id),
        "string", toCStr(path_in_repo));
    if (error) {
        qWarning("failed to mark file lock state: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    g_free (ret);
    return 0;
}


bool SeafileRpcClient::setServerProperty(const QString &url,
                                         const QString &key,
                                         const QString &value)
{
    // printf("set server config: %s %s = %s\n", toCStr(url), toCStr(key),
    //        toCStr(value));
    GError *error = NULL;
    searpc_client_call__int(seadrive_rpc_client_, "seafile_set_server_property",
                            &error, 3, "string", toCStr(url), "string",
                            toCStr(key), "string", toCStr(value));
    if (error) {
        qWarning("Unable to set server property %s %s", toCStr(url),
                 toCStr(key));
        g_error_free(error);
        return false;
    }
    return true;
}


bool SeafileRpcClient::switchAccount(const QString &server,
                                     const QString &username,
                                     const QString &token,
                                     bool is_pro)
{
    GError *error = NULL;
    searpc_client_call__int(seadrive_rpc_client_,
                            "seafile_switch_account",
                            &error,
                            4,
                            "string",
                            toCStr(server),
                            "string",
                            toCStr(username),
                            "string",
                            toCStr(token),
                            "int",
                            is_pro ? 1 : 0);
    if (error) {
        qWarning("Unable to switch account %s %s: %s",
                 toCStr(server),
                 toCStr(username),
                 error->message ? error->message : "");
        g_error_free(error);
        return false;
    }
    return true;
}