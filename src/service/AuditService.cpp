#include "services/AuditService.hpp"

namespace Banking {

AuditService::AuditService(AuditRepository& auditRepo, UserRepository& userRepo, SessionManager& sessionManager)
    : m_auditRepo(auditRepo), m_userRepo(userRepo), m_sessionManager(sessionManager) {}

GetAuditLogsResult AuditService::getLogs(const GetAuditLogsRequest& req) {
    GetAuditLogsResult res;

    // 1. Xác thực phiên làm việc
    auto sessOpt = m_sessionManager.getSession(req.session_token);
    if (!sessOpt.has_value()) {
        res.message = "Phiên làm việc đã hết hạn hoặc không hợp lệ!";
        return res;
    }

    // 2. Kiểm tra RBAC: Chỉ Admin mới có quyền xem Audit Log
    auto user = m_userRepo.findById(sessOpt->user_id);
    if (!user || !user->hasPermission(Permission::ADMIN_VIEW_AUDIT_LOG)) {
        res.message = "BẢO MẬT: Bạn không có quyền truy cập nhật ký hệ thống (Audit Log)!";
        return res;
    }

    // 3. Truy vấn phân trang từ Repository
    return m_auditRepo.getLogsPaginated(req.page, req.page_size, req.operator_filter);
}

} // namespace Banking